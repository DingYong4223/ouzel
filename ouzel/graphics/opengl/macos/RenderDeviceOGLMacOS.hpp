// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_GRAPHICS_RENDERDEVICEOGLMACOS_HPP
#define OUZEL_GRAPHICS_RENDERDEVICEOGLMACOS_HPP

#include "core/Setup.h"

#if OUZEL_PLATFORM_MACOS && OUZEL_COMPILE_OPENGL

#if defined(__OBJC__)
#import <CoreVideo/CoreVideo.h>
#import <AppKit/NSOpenGL.h>
typedef NSOpenGLContext* NSOpenGLContextPtr;
typedef NSOpenGLPixelFormat* NSOpenGLPixelFormatPtr;
#else
#include <objc/objc.h>
typedef void* CVDisplayLinkRef;
typedef id NSOpenGLContextPtr;
typedef id NSOpenGLPixelFormatPtr;
#endif

#include "graphics/opengl/RenderDeviceOGL.hpp"
#include "events/EventHandler.hpp"

namespace ouzel
{
    namespace graphics
    {
        class RenderDeviceOGLMacOS final: public RenderDeviceOGL
        {
            friend Renderer;
        public:
            RenderDeviceOGLMacOS(const std::function<void(const Event&)>& initCallback);
            ~RenderDeviceOGLMacOS();

            std::vector<Size2> getSupportedResolutions() const override;

            inline NSOpenGLContextPtr getOpenGLContext() const { return openGLContext; }

            void renderCallback();

        protected:
            void init(Window* newWindow,
                      const Size2& newSize,
                      uint32_t newSampleCount,
                      Texture::Filter newTextureFilter,
                      uint32_t newMaxAnisotropy,
                      bool newVerticalSync,
                      bool newDepth,
                      bool newDebugRenderer) override;

            void setSize(const Size2& newSize) override;
            void present() override;

            bool handleWindow(const WindowEvent& event);

            NSOpenGLContextPtr openGLContext = nil;
            NSOpenGLPixelFormatPtr pixelFormat = nil;

            CVDisplayLinkRef displayLink = nullptr;
            EventHandler eventHandler;

            std::atomic_bool running;
        };
    } // namespace graphics
} // namespace ouzel

#endif

#endif // OUZEL_GRAPHICS_RENDERDEVICEOGLMACOS_HPP
