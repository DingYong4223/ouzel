// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#include "core/Setup.h"

#if OUZEL_PLATFORM_ANDROID && OUZEL_COMPILE_OPENGL

#include "RenderDeviceOGLAndroid.hpp"
#include "core/Engine.hpp"
#include "core/Window.hpp"
#include "core/android/NativeWindowAndroid.hpp"
#include "utils/Errors.hpp"
#include "utils/Log.hpp"
#include "utils/Utils.hpp"

namespace ouzel
{
    namespace graphics
    {
        RenderDeviceOGLAndroid::RenderDeviceOGLAndroid(const std::function<void(const Event&)>& initCallback):
            RenderDeviceOGL(initCallback)
        {
        }

        RenderDeviceOGLAndroid::~RenderDeviceOGLAndroid()
        {
            running = false;
            CommandBuffer commandBuffer;
            commandBuffer.commands.push(std::unique_ptr<Command>(new PresentCommand()));
            submitCommandBuffer(std::move(commandBuffer));

            if (renderThread.joinable()) renderThread.join();

            if (context)
            {
                eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                eglDestroyContext(display, context);
            }

            if (surface)
                eglDestroySurface(display, surface);

            if (display)
                eglTerminate(display);
        }

        void RenderDeviceOGLAndroid::init(Window* newWindow,
                                          const Size2&,
                                          uint32_t newSampleCount,
                                          Texture::Filter newTextureFilter,
                                          uint32_t newMaxAnisotropy,
                                          bool newVerticalSync,
                                          bool newDepth,
                                          bool newDebugRenderer)
        {
            display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

            if (!display)
                throw SystemError("Failed to get display");

            if (!eglInitialize(display, nullptr, nullptr))
                throw SystemError("Failed to initialize EGL");

            const EGLint attributeList[] =
            {
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_DEPTH_SIZE, newDepth ? 24 : 0,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_SAMPLE_BUFFERS, (newSampleCount > 1) ? 1 : 0,
                EGL_SAMPLES, static_cast<int>(newSampleCount),
                EGL_NONE
            };
            EGLConfig config;
            EGLint numConfig;
            if (!eglChooseConfig(display, attributeList, &config, 1, &numConfig))
                throw SystemError("Failed to choose EGL config");

            if (!eglBindAPI(EGL_OPENGL_ES_API))
                throw SystemError("Failed to bind OpenGL ES API");

            EGLint format;
            if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format))
                throw SystemError("Failed to get config attribute " + std::to_string(eglGetError()));

            NativeWindowAndroid* windowAndroid = static_cast<NativeWindowAndroid*>(newWindow->getNativeWindow());

            ANativeWindow_setBuffersGeometry(windowAndroid->getNativeWindow(), 0, 0, format);

            surface = eglCreateWindowSurface(display, config, windowAndroid->getNativeWindow(), nullptr);
            if (surface == EGL_NO_SURFACE)
                throw SystemError("Failed to create EGL window surface, error: " + std::to_string(eglGetError()));

            for (EGLint version = 3; version >= 2; --version)
            {
                std::vector<EGLint> contextAttributes =
                {
                    EGL_CONTEXT_CLIENT_VERSION, version
                };

                if (newDebugRenderer)
                {
                    contextAttributes.push_back(EGL_CONTEXT_FLAGS_KHR);
                    contextAttributes.push_back(EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR);
                }

                contextAttributes.push_back(EGL_NONE);

                context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes.data());

                if (context != EGL_NO_CONTEXT)
                {
                    apiMajorVersion = version;
                    apiMinorVersion = 0;
                    engine->log(Log::Level::INFO) << "EGL OpenGL ES " << version << " context created";
                    break;
                }
            }

            if (context == EGL_NO_CONTEXT)
                throw SystemError("Failed to create EGL context");

            if (!eglMakeCurrent(display, surface, surface, context))
                throw SystemError("Failed to set current EGL context");

            if (!eglSwapInterval(display, newVerticalSync ? 1 : 0))
                throw SystemError("Failed to set EGL frame interval");

            EGLint surfaceWidth;
            EGLint surfaceHeight;

            if (!eglQuerySurface(display, surface, EGL_WIDTH, &surfaceWidth) ||
                !eglQuerySurface(display, surface, EGL_HEIGHT, &surfaceHeight))
                throw SystemError("Failed to get query window size, error: " + std::to_string(eglGetError()));

            frameBufferWidth = surfaceWidth;
            frameBufferHeight = surfaceHeight;

            Size2 backBufferSize = Size2(static_cast<float>(frameBufferWidth),
                                         static_cast<float>(frameBufferHeight));

            RenderDeviceOGL::init(newWindow,
                                  backBufferSize,
                                  newSampleCount,
                                  newTextureFilter,
                                  newMaxAnisotropy,
                                  newVerticalSync,
                                  newDepth,
                                  newDebugRenderer);

            if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
                throw SystemError("Failed to unset EGL context");

            running = true;
            renderThread = std::thread(&RenderDeviceOGLAndroid::main, this);
        }

        void RenderDeviceOGLAndroid::reload()
        {
            running = false;
            CommandBuffer commandBuffer;
            commandBuffer.commands.push(std::unique_ptr<Command>(new PresentCommand()));
            submitCommandBuffer(std::move(commandBuffer));

            if (renderThread.joinable()) renderThread.join();

            const EGLint attributeList[] =
            {
                EGL_RED_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_BLUE_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_DEPTH_SIZE, depth ? 24 : 0,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_SAMPLE_BUFFERS, (sampleCount > 1) ? 1 : 0,
                EGL_SAMPLES, static_cast<int>(sampleCount),
                EGL_NONE
            };
            EGLConfig config;
            EGLint numConfig;
            if (!eglChooseConfig(display, attributeList, &config, 1, &numConfig))
                throw SystemError("Failed to choose EGL config");

            if (!eglBindAPI(EGL_OPENGL_ES_API))
                throw SystemError("Failed to bind OpenGL ES API");

            EGLint format;
            if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format))
                throw SystemError("Failed to get config attribute, error: " + std::to_string(eglGetError()));

            NativeWindowAndroid* windowAndroid = static_cast<NativeWindowAndroid*>(window->getNativeWindow());

            ANativeWindow_setBuffersGeometry(windowAndroid->getNativeWindow(), 0, 0, format);

            surface = eglCreateWindowSurface(display, config, windowAndroid->getNativeWindow(), nullptr);
            if (surface == EGL_NO_SURFACE)
                throw SystemError("Failed to create EGL window surface, error: " + std::to_string(eglGetError()));

            for (EGLint version = 3; version >= 2; --version)
            {
                std::vector<EGLint> contextAttributes =
                {
                    EGL_CONTEXT_CLIENT_VERSION, version
                };

                if (debugRenderer)
                {
                    contextAttributes.push_back(EGL_CONTEXT_FLAGS_KHR);
                    contextAttributes.push_back(EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR);
                }

                contextAttributes.push_back(EGL_NONE);

                context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes.data());

                if (context != EGL_NO_CONTEXT)
                {
                    apiMajorVersion = version;
                    apiMinorVersion = 0;
                    engine->log(Log::Level::INFO) << "EGL OpenGL ES " << version << " context created";
                    break;
                }
            }

            if (context == EGL_NO_CONTEXT)
                throw SystemError("Failed to create EGL context");

            if (!eglMakeCurrent(display, surface, surface, context))
                throw SystemError("Failed to set current EGL context");

            if (!eglSwapInterval(display, verticalSync ? 1 : 0))
                throw SystemError("Failed to set EGL frame interval");

            EGLint surfaceWidth;
            EGLint surfaceHeight;

            if (!eglQuerySurface(display, surface, EGL_WIDTH, &surfaceWidth) ||
                !eglQuerySurface(display, surface, EGL_HEIGHT, &surfaceHeight))
                throw SystemError("Failed to get query window size " + std::to_string(eglGetError()));

            frameBufferWidth = surfaceWidth;
            frameBufferHeight = surfaceHeight;

            stateCache = StateCache();

            glDisableProc(GL_DITHER);
            glDepthFunc(GL_LEQUAL);

            GLenum error;

            if ((error = glGetErrorProc()) != GL_NO_ERROR)
                throw SystemError("Failed to set depth function, error: " + std::to_string(error));

            if (glGenVertexArraysProc) glGenVertexArraysProc(1, &vertexArrayId);

            for (const std::unique_ptr<RenderResourceOGL>& resource : resources)
                if (resource)
                    static_cast<RenderResourceOGL*>(resource.get())->reload();

            if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
                throw SystemError("Failed to unset EGL context");

            running = true;
            renderThread = std::thread(&RenderDeviceOGLAndroid::main, this);
        }

        void RenderDeviceOGLAndroid::destroy()
        {
            running = false;
            CommandBuffer commandBuffer;
            commandBuffer.commands.push(std::unique_ptr<Command>(new PresentCommand()));
            submitCommandBuffer(std::move(commandBuffer));
            
            if (renderThread.joinable()) renderThread.join();

            if (context)
            {
                if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
                    engine->log(Log::Level::ERR) << "Failed to unset EGL context";

                if (!eglDestroyContext(display, context))
                    engine->log(Log::Level::ERR) << "Failed to destroy EGL context";

                context = nullptr;
            }

            if (surface)
            {
                if (!eglDestroySurface(display, surface))
                    engine->log(Log::Level::ERR) << "Failed to destroy EGL surface";

                surface = nullptr;
            }
        }

        void RenderDeviceOGLAndroid::present()
        {
            if (eglSwapBuffers(display, surface) != EGL_TRUE)
                throw SystemError("Failed to swap buffers, error: " + std::to_string(eglGetError()));
        }

        void RenderDeviceOGLAndroid::main()
        {
            setCurrentThreadName("Render");

            if (!eglMakeCurrent(display, surface, surface, context))
                throw SystemError("Failed to set current EGL context, error: " + std::to_string(eglGetError()));

            while (running)
            {
                try
                {
                    process();
                }
                catch (const std::exception& e)
                {
                    engine->log(Log::Level::ERR) << e.what();
                }
            }

            if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT))
                engine->log(Log::Level::ERR) << "Failed to unset EGL context, error: " << eglGetError();
        }
    } // namespace graphics
} // namespace ouzel

#endif
