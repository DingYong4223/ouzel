// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#include "Setup.h"
#include <algorithm>
#include "Engine.hpp"
#include "utils/INI.hpp"
#include "utils/Errors.hpp"
#include "utils/Log.hpp"
#include "utils/Utils.hpp"
#include "graphics/Renderer.hpp"
#include "audio/Audio.hpp"

#if OUZEL_COMPILE_OPENGL
#  if OUZEL_SUPPORTS_OPENGLES
#    include "opengl/ColorPSGLES2.h"
#    include "opengl/ColorVSGLES2.h"
#    include "opengl/TexturePSGLES2.h"
#    include "opengl/TextureVSGLES2.h"
#    include "opengl/ColorPSGLES3.h"
#    include "opengl/ColorVSGLES3.h"
#    include "opengl/TexturePSGLES3.h"
#    include "opengl/TextureVSGLES3.h"
#  else
#    include "opengl/ColorPSGL2.h"
#    include "opengl/ColorVSGL2.h"
#    include "opengl/TexturePSGL2.h"
#    include "opengl/TextureVSGL2.h"
#    include "opengl/ColorPSGL3.h"
#    include "opengl/ColorVSGL3.h"
#    include "opengl/TexturePSGL3.h"
#    include "opengl/TextureVSGL3.h"
#    include "opengl/ColorPSGL4.h"
#    include "opengl/ColorVSGL4.h"
#    include "opengl/TexturePSGL4.h"
#    include "opengl/TextureVSGL4.h"
#  endif
#endif

#if OUZEL_COMPILE_DIRECT3D11
#  include "direct3d11/TexturePSD3D11.h"
#  include "direct3d11/TextureVSD3D11.h"
#  include "direct3d11/ColorPSD3D11.h"
#  include "direct3d11/ColorVSD3D11.h"
#endif

#if OUZEL_COMPILE_METAL
#  if OUZEL_PLATFORM_MACOS
#    include "metal/ColorPSMacOS.h"
#    include "metal/ColorVSMacOS.h"
#    include "metal/TexturePSMacOS.h"
#    include "metal/TextureVSMacOS.h"
#    define COLOR_PIXEL_SHADER_METAL ColorPSMacOS_metallib
#    define COLOR_VERTEX_SHADER_METAL ColorVSMacOS_metallib
#    define TEXTURE_PIXEL_SHADER_METAL TexturePSMacOS_metallib
#    define TEXTURE_VERTEX_SHADER_METAL TextureVSMacOS_metallib
#  elif OUZEL_PLATFORM_TVOS
#    include "metal/ColorPSTVOS.h"
#    include "metal/ColorVSTVOS.h"
#    include "metal/TexturePSTVOS.h"
#    include "metal/TextureVSTVOS.h"
#    define COLOR_PIXEL_SHADER_METAL ColorPSTVOS_metallib
#    define COLOR_VERTEX_SHADER_METAL ColorVSTVOS_metallib
#    define TEXTURE_PIXEL_SHADER_METAL TexturePSTVOS_metallib
#    define TEXTURE_VERTEX_SHADER_METAL TextureVSTVOS_metallib
#  elif OUZEL_PLATFORM_IOS
#    include "metal/ColorPSIOS.h"
#    include "metal/ColorVSIOS.h"
#    include "metal/TexturePSIOS.h"
#    include "metal/TextureVSIOS.h"
#    define COLOR_PIXEL_SHADER_METAL ColorPSIOS_metallib
#    define COLOR_VERTEX_SHADER_METAL ColorVSIOS_metallib
#    define TEXTURE_PIXEL_SHADER_METAL TexturePSIOS_metallib
#    define TEXTURE_VERTEX_SHADER_METAL TextureVSIOS_metallib
#  endif
#endif

namespace ouzel
{
    Engine* engine = nullptr;

    Engine::Engine():
#ifdef DEBUG
        logThreshold(Log::Level::ALL),
#else
        logThreshold(Log::Level::INFO),
#endif
        fileSystem(*this), assetBundle(cache, fileSystem),
        active(false), paused(false),
        oneUpdatePerFrame(false), screenSaverEnabled(true)
    {
        engine = this;
    }

    Engine::~Engine()
    {
        if (active)
        {
            std::unique_ptr<SystemEvent> event(new SystemEvent());
            event->type = Event::Type::ENGINE_STOP;
            eventDispatcher.postEvent(std::move(event));
        }

        paused = true;
        active = false;

#if OUZEL_MULTITHREADED
        if (updateThread.joinable())
        {
            std::unique_lock<std::mutex> lock(updateMutex);
            updateCondition.notify_all();
            lock.unlock();
            updateThread.join();
        }
#endif

        engine = nullptr;
    }

    void Engine::init()
    {
        setCurrentThreadName("Main");

        graphics::Driver graphicsDriver = graphics::Driver::DEFAULT;
        Size2 size;
        uint32_t sampleCount = 1; // MSAA sample count
        graphics::Texture::Filter textureFilter = graphics::Texture::Filter::POINT;
        uint32_t maxAnisotropy = 1;
        bool resizable = false;
        bool fullscreen = false;
        bool verticalSync = true;
        bool depth = false;
        bool debugRenderer = false;
        bool exclusiveFullscreen = false;
        bool highDpi = true; // should high DPI resolution be used
        audio::Driver audioDriver = audio::Driver::DEFAULT;
        bool debugAudio = false;

        defaultSettings = ini::Data(fileSystem.readFile("settings.ini"));

        try
        {
            userSettings = ini::Data(fileSystem.readFile(fileSystem.getStorageDirectory() + FileSystem::DIRECTORY_SEPARATOR + "settings.ini"));
        }
        catch (const FileError&)
        {
            engine->log(Log::Level::INFO) << "User settings not provided";
        }

        const ini::Section& userEngineSection = userSettings.getSection("engine");
        const ini::Section& defaultEngineSection = defaultSettings.getSection("engine");

        std::string graphicsDriverValue = userEngineSection.getValue("graphicsDriver", defaultEngineSection.getValue("graphicsDriver"));

        if (!graphicsDriverValue.empty())
        {
            if (graphicsDriverValue == "default")
                graphicsDriver = graphics::Driver::DEFAULT;
            else if (graphicsDriverValue == "empty")
                graphicsDriver = graphics::Driver::EMPTY;
            else if (graphicsDriverValue == "opengl")
                graphicsDriver = graphics::Driver::OPENGL;
            else if (graphicsDriverValue == "direct3d11")
                graphicsDriver = graphics::Driver::DIRECT3D11;
            else if (graphicsDriverValue == "metal")
                graphicsDriver = graphics::Driver::METAL;
            else
                throw ConfigError("Invalid graphics driver specified");
        }

        std::string widthValue = userEngineSection.getValue("width", defaultEngineSection.getValue("width"));
        if (!widthValue.empty()) size.width = std::stof(widthValue);

        std::string heightValue = userEngineSection.getValue("height", defaultEngineSection.getValue("height"));
        if (!heightValue.empty()) size.height = std::stof(heightValue);

        std::string sampleCountValue = userEngineSection.getValue("sampleCount", defaultEngineSection.getValue("sampleCount"));
        if (!sampleCountValue.empty()) sampleCount = static_cast<uint32_t>(std::stoul(sampleCountValue));

        std::string textureFilterValue = userEngineSection.getValue("textureFilter", defaultEngineSection.getValue("textureFilter"));
        if (!textureFilterValue.empty())
        {
            if (textureFilterValue == "point")
                textureFilter = graphics::Texture::Filter::POINT;
            else if (textureFilterValue == "linear")
                textureFilter = graphics::Texture::Filter::LINEAR;
            else if (textureFilterValue == "bilinear")
                textureFilter = graphics::Texture::Filter::BILINEAR;
            else if (textureFilterValue == "trilinear")
                textureFilter = graphics::Texture::Filter::TRILINEAR;
            else
                throw ConfigError("Invalid texture filter specified");
        }

        std::string maxAnisotropyValue = userEngineSection.getValue("maxAnisotropy", defaultEngineSection.getValue("maxAnisotropy"));
        if (!maxAnisotropyValue.empty()) maxAnisotropy = static_cast<uint32_t>(std::stoul(maxAnisotropyValue));

        std::string resizableValue = userEngineSection.getValue("resizable", defaultEngineSection.getValue("resizable"));
        if (!resizableValue.empty()) resizable = (resizableValue == "true" || resizableValue == "1" || resizableValue == "yes");

        std::string fullscreenValue = userEngineSection.getValue("fullscreen", defaultEngineSection.getValue("fullscreen"));
        if (!fullscreenValue.empty()) fullscreen = (fullscreenValue == "true" || fullscreenValue == "1" || fullscreenValue == "yes");

        std::string verticalSyncValue = userEngineSection.getValue("verticalSync", defaultEngineSection.getValue("verticalSync"));
        if (!verticalSyncValue.empty()) verticalSync = (verticalSyncValue == "true" || verticalSyncValue == "1" || verticalSyncValue == "yes");

        std::string exclusiveFullscreenValue = userEngineSection.getValue("exclusiveFullscreen", defaultEngineSection.getValue("exclusiveFullscreen"));
        if (!exclusiveFullscreenValue.empty()) exclusiveFullscreen = (exclusiveFullscreenValue == "true" || exclusiveFullscreenValue == "1" || exclusiveFullscreenValue == "yes");

        std::string depthValue = userEngineSection.getValue("depth", defaultEngineSection.getValue("depth"));
        if (!depthValue.empty()) depth = (depthValue == "true" || depthValue == "1" || depthValue == "yes");

        std::string debugRendererValue = userEngineSection.getValue("debugRenderer", defaultEngineSection.getValue("debugRenderer"));
        if (!debugRendererValue.empty()) debugRenderer = (debugRendererValue == "true" || debugRendererValue == "1" || debugRendererValue == "yes");

        std::string highDpiValue = userEngineSection.getValue("highDpi", defaultEngineSection.getValue("highDpi"));
        if (!highDpiValue.empty()) highDpi = (highDpiValue == "true" || highDpiValue == "1" || highDpiValue == "yes");

        std::string audioDriverValue = userEngineSection.getValue("audioDriver", defaultEngineSection.getValue("audioDriver"));

        if (!audioDriverValue.empty())
        {
            if (audioDriverValue == "default")
                audioDriver = audio::Driver::DEFAULT;
            else if (audioDriverValue == "empty")
                audioDriver = audio::Driver::EMPTY;
            else if (audioDriverValue == "openal")
                audioDriver = audio::Driver::OPENAL;
            else if (audioDriverValue == "directsound")
                audioDriver = audio::Driver::DIRECTSOUND;
            else if (audioDriverValue == "xaudio2")
                audioDriver = audio::Driver::XAUDIO2;
            else if (audioDriverValue == "opensl")
                audioDriver = audio::Driver::OPENSL;
            else if (audioDriverValue == "coreaudio")
                audioDriver = audio::Driver::COREAUDIO;
            else if (audioDriverValue == "alsa")
                audioDriver = audio::Driver::ALSA;
            else
                throw ConfigError("Invalid audio driver specified");
        }

        std::string debugAudioValue = userEngineSection.getValue("debugAudio", defaultEngineSection.getValue("debugAudio"));
        if (!debugAudioValue.empty()) debugAudio = (debugAudioValue == "true" || debugAudioValue == "1" || debugAudioValue == "yes");

        if (graphicsDriver == graphics::Driver::DEFAULT)
        {
            auto availableDrivers = graphics::Renderer::getAvailableRenderDrivers();

            if (availableDrivers.find(graphics::Driver::METAL) != availableDrivers.end())
                graphicsDriver = graphics::Driver::METAL;
            else if (availableDrivers.find(graphics::Driver::DIRECT3D11) != availableDrivers.end())
                graphicsDriver = graphics::Driver::DIRECT3D11;
            else if (availableDrivers.find(graphics::Driver::OPENGL) != availableDrivers.end())
                graphicsDriver = graphics::Driver::OPENGL;
            else
                graphicsDriver = graphics::Driver::EMPTY;
        }

        window.reset(new Window(size,
                                resizable,
                                fullscreen,
                                exclusiveFullscreen,
                                OUZEL_APPLICATION_NAME,
                                graphicsDriver,
                                highDpi,
                                depth));

        renderer.reset(new graphics::Renderer(graphicsDriver,
                                              window.get(),
                                              window->getResolution(),
                                              sampleCount,
                                              textureFilter,
                                              maxAnisotropy,
                                              verticalSync,
                                              depth,
                                              debugRenderer));

        if (audioDriver == audio::Driver::DEFAULT)
        {
            auto availableDrivers = audio::Audio::getAvailableAudioDrivers();

            if (availableDrivers.find(audio::Driver::COREAUDIO) != availableDrivers.end())
                audioDriver = audio::Driver::COREAUDIO;
            else if (availableDrivers.find(audio::Driver::ALSA) != availableDrivers.end())
                audioDriver = audio::Driver::ALSA;
            else if (availableDrivers.find(audio::Driver::OPENAL) != availableDrivers.end())
                audioDriver = audio::Driver::OPENAL;
            else if (availableDrivers.find(audio::Driver::XAUDIO2) != availableDrivers.end())
                audioDriver = audio::Driver::XAUDIO2;
            else if (availableDrivers.find(audio::Driver::DIRECTSOUND) != availableDrivers.end())
                audioDriver = audio::Driver::DIRECTSOUND;
            else if (availableDrivers.find(audio::Driver::OPENSL) != availableDrivers.end())
                audioDriver = audio::Driver::OPENSL;
            else
                audioDriver = audio::Driver::EMPTY;
        }

        audio.reset(new audio::Audio(audioDriver, debugAudio, window.get()));

        inputManager.reset(new input::InputManager());

        // default assets
        switch (graphicsDriver)
        {
#if OUZEL_COMPILE_OPENGL
            case graphics::Driver::OPENGL:
            {
                std::shared_ptr<graphics::Shader> textureShader = std::make_shared<graphics::Shader>(*renderer);

                switch (renderer->getDevice()->getAPIMajorVersion())
                {
#  if OUZEL_SUPPORTS_OPENGLES
                    case 2:
                        textureShader->init(std::vector<uint8_t>(std::begin(TexturePSGLES2_glsl), std::end(TexturePSGLES2_glsl)),
                                            std::vector<uint8_t>(std::begin(TextureVSGLES2_glsl), std::end(TextureVSGLES2_glsl)),
                                            {graphics::Vertex::Attribute::Usage::POSITION, graphics::Vertex::Attribute::Usage::COLOR, graphics::Vertex::Attribute::Usage::TEXTURE_COORDINATES0},
                                            {{"color", graphics::DataType::FLOAT_VECTOR4}},
                                            {{"modelViewProj", graphics::DataType::FLOAT_MATRIX4}});
                        break;
                    case 3:
                        textureShader->init(std::vector<uint8_t>(std::begin(TexturePSGLES3_glsl), std::end(TexturePSGLES3_glsl)),
                                            std::vector<uint8_t>(std::begin(TextureVSGLES3_glsl), std::end(TextureVSGLES3_glsl)),
                                            {graphics::Vertex::Attribute::Usage::POSITION, graphics::Vertex::Attribute::Usage::COLOR, graphics::Vertex::Attribute::Usage::TEXTURE_COORDINATES0},
                                            {{"color", graphics::DataType::FLOAT_VECTOR4}},
                                            {{"modelViewProj", graphics::DataType::FLOAT_MATRIX4}});
                        break;
#  else
                    case 2:
                        textureShader->init(std::vector<uint8_t>(std::begin(TexturePSGL2_glsl), std::end(TexturePSGL2_glsl)),
                                            std::vector<uint8_t>(std::begin(TextureVSGL2_glsl), std::end(TextureVSGL2_glsl)),
                                            {graphics::Vertex::Attribute::Usage::POSITION, graphics::Vertex::Attribute::Usage::COLOR, graphics::Vertex::Attribute::Usage::TEXTURE_COORDINATES0},
                                            {{"color", graphics::DataType::FLOAT_VECTOR4}},
                                            {{"modelViewProj", graphics::DataType::FLOAT_MATRIX4}});
                        break;
                    case 3:
                        textureShader->init(std::vector<uint8_t>(std::begin(TexturePSGL3_glsl), std::end(TexturePSGL3_glsl)),
                                            std::vector<uint8_t>(std::begin(TextureVSGL3_glsl), std::end(TextureVSGL3_glsl)),
                                            {graphics::Vertex::Attribute::Usage::POSITION, graphics::Vertex::Attribute::Usage::COLOR, graphics::Vertex::Attribute::Usage::TEXTURE_COORDINATES0},
                                            {{"color", graphics::DataType::FLOAT_VECTOR4}},
                                            {{"modelViewProj", graphics::DataType::FLOAT_MATRIX4}});
                        break;
                    case 4:
                        textureShader->init(std::vector<uint8_t>(std::begin(TexturePSGL4_glsl), std::end(TexturePSGL4_glsl)),
                                            std::vector<uint8_t>(std::begin(TextureVSGL4_glsl), std::end(TextureVSGL4_glsl)),
                                            {graphics::Vertex::Attribute::Usage::POSITION, graphics::Vertex::Attribute::Usage::COLOR, graphics::Vertex::Attribute::Usage::TEXTURE_COORDINATES0},
                                            {{"color", graphics::DataType::FLOAT_VECTOR4}},
                                            {{"modelViewProj", graphics::DataType::FLOAT_MATRIX4}});
                        break;
#  endif
                    default:
                        throw SystemError("Unsupported OpenGL version");
                }

                assetBundle.setShader(SHADER_TEXTURE, textureShader);

                std::shared_ptr<graphics::Shader> colorShader = std::make_shared<graphics::Shader>(*renderer);

                switch (renderer->getDevice()->getAPIMajorVersion())
                {
#  if OUZEL_SUPPORTS_OPENGLES
                    case 2:
                        colorShader->init(std::vector<uint8_t>(std::begin(ColorPSGLES2_glsl), std::end(ColorPSGLES2_glsl)),
                                          std::vector<uint8_t>(std::begin(ColorVSGLES2_glsl), std::end(ColorVSGLES2_glsl)),
                                          {graphics::Vertex::Attribute::Usage::POSITION, graphics::Vertex::Attribute::Usage::COLOR},
                                          {{"color", graphics::DataType::FLOAT_VECTOR4}},
                                          {{"modelViewProj", graphics::DataType::FLOAT_MATRIX4}});

                        break;
                    case 3:
                        colorShader->init(std::vector<uint8_t>(std::begin(ColorPSGLES3_glsl), std::end(ColorPSGLES3_glsl)),
                                          std::vector<uint8_t>(std::begin(ColorVSGLES3_glsl), std::end(ColorVSGLES3_glsl)),
                                          {graphics::Vertex::Attribute::Usage::POSITION, graphics::Vertex::Attribute::Usage::COLOR},
                                          {{"color", graphics::DataType::FLOAT_VECTOR4}},
                                          {{"modelViewProj", graphics::DataType::FLOAT_MATRIX4}});
                        break;
#  else
                    case 2:
                        colorShader->init(std::vector<uint8_t>(std::begin(ColorPSGL2_glsl), std::end(ColorPSGL2_glsl)),
                                          std::vector<uint8_t>(std::begin(ColorVSGL2_glsl), std::end(ColorVSGL2_glsl)),
                                          {graphics::Vertex::Attribute::Usage::POSITION, graphics::Vertex::Attribute::Usage::COLOR},
                                          {{"color", graphics::DataType::FLOAT_VECTOR4}},
                                          {{"modelViewProj", graphics::DataType::FLOAT_MATRIX4}});
                        break;
                    case 3:
                        colorShader->init(std::vector<uint8_t>(std::begin(ColorPSGL3_glsl), std::end(ColorPSGL3_glsl)),
                                          std::vector<uint8_t>(std::begin(ColorVSGL3_glsl), std::end(ColorVSGL3_glsl)),
                                          {graphics::Vertex::Attribute::Usage::POSITION, graphics::Vertex::Attribute::Usage::COLOR},
                                          {{"color", graphics::DataType::FLOAT_VECTOR4}},
                                          {{"modelViewProj", graphics::DataType::FLOAT_MATRIX4}});
                        break;
                    case 4:
                        colorShader->init(std::vector<uint8_t>(std::begin(ColorPSGL4_glsl), std::end(ColorPSGL4_glsl)),
                                          std::vector<uint8_t>(std::begin(ColorVSGL4_glsl), std::end(ColorVSGL4_glsl)),
                                          {graphics::Vertex::Attribute::Usage::POSITION, graphics::Vertex::Attribute::Usage::COLOR},
                                          {{"color", graphics::DataType::FLOAT_VECTOR4}},
                                          {{"modelViewProj", graphics::DataType::FLOAT_MATRIX4}});
                        break;
#  endif
                    default:
                        throw SystemError("Unsupported OpenGL version");
                }

                assetBundle.setShader(SHADER_COLOR, colorShader);
                break;
            }
#endif

#if OUZEL_COMPILE_DIRECT3D11
            case graphics::Driver::DIRECT3D11:
            {
                std::shared_ptr<graphics::Shader> textureShader = std::make_shared<graphics::Shader>(*renderer);
                textureShader->init(std::vector<uint8_t>(std::begin(TEXTURE_PIXEL_SHADER_D3D11), std::end(TEXTURE_PIXEL_SHADER_D3D11)),
                                    std::vector<uint8_t>(std::begin(TEXTURE_VERTEX_SHADER_D3D11), std::end(TEXTURE_VERTEX_SHADER_D3D11)),
                                    {graphics::Vertex::Attribute::Usage::POSITION, graphics::Vertex::Attribute::Usage::COLOR, graphics::Vertex::Attribute::Usage::TEXTURE_COORDINATES0},
                                    {{"color", graphics::DataType::FLOAT_VECTOR4}},
                                    {{"modelViewProj", graphics::DataType::FLOAT_MATRIX4}});

                assetBundle.setShader(SHADER_TEXTURE, textureShader);

                std::shared_ptr<graphics::Shader> colorShader = std::make_shared<graphics::Shader>(*renderer);
                colorShader->init(std::vector<uint8_t>(std::begin(COLOR_PIXEL_SHADER_D3D11), std::end(COLOR_PIXEL_SHADER_D3D11)),
                                  std::vector<uint8_t>(std::begin(COLOR_VERTEX_SHADER_D3D11), std::end(COLOR_VERTEX_SHADER_D3D11)),
                                  {graphics::Vertex::Attribute::Usage::POSITION, graphics::Vertex::Attribute::Usage::COLOR},
                                  {{"color", graphics::DataType::FLOAT_VECTOR4}},
                                  {{"modelViewProj", graphics::DataType::FLOAT_MATRIX4}});

                assetBundle.setShader(SHADER_COLOR, colorShader);
                break;
            }
#endif

#if OUZEL_COMPILE_METAL
            case graphics::Driver::METAL:
            {
                std::shared_ptr<graphics::Shader> textureShader = std::make_shared<graphics::Shader>(*renderer);
                textureShader->init(std::vector<uint8_t>(std::begin(TEXTURE_PIXEL_SHADER_METAL), std::end(TEXTURE_PIXEL_SHADER_METAL)),
                                    std::vector<uint8_t>(std::begin(TEXTURE_VERTEX_SHADER_METAL), std::end(TEXTURE_VERTEX_SHADER_METAL)),
                                    {graphics::Vertex::Attribute::Usage::POSITION, graphics::Vertex::Attribute::Usage::COLOR, graphics::Vertex::Attribute::Usage::TEXTURE_COORDINATES0},
                                    {{"color", graphics::DataType::FLOAT_VECTOR4}},
                                    {{"modelViewProj", graphics::DataType::FLOAT_MATRIX4}},
                                    256, 256,
                                    "mainPS", "mainVS");

                assetBundle.setShader(SHADER_TEXTURE, textureShader);

                std::shared_ptr<graphics::Shader> colorShader = std::make_shared<graphics::Shader>(*renderer);
                colorShader->init(std::vector<uint8_t>(std::begin(COLOR_PIXEL_SHADER_METAL), std::end(COLOR_PIXEL_SHADER_METAL)),
                                  std::vector<uint8_t>(std::begin(COLOR_VERTEX_SHADER_METAL), std::end(COLOR_VERTEX_SHADER_METAL)),
                                  {graphics::Vertex::Attribute::Usage::POSITION, graphics::Vertex::Attribute::Usage::COLOR},
                                  {{"color", graphics::DataType::FLOAT_VECTOR4}},
                                  {{"modelViewProj", graphics::DataType::FLOAT_MATRIX4}},
                                  256, 256,
                                  "mainPS", "mainVS");

                assetBundle.setShader(SHADER_COLOR, colorShader);
                break;
            }
#endif

            default:
            {
                std::shared_ptr<graphics::Shader> textureShader = std::make_shared<graphics::Shader>(*renderer);

                textureShader->init(std::vector<uint8_t>(),
                                    std::vector<uint8_t>(),
                                    {graphics::Vertex::Attribute::Usage::POSITION, graphics::Vertex::Attribute::Usage::COLOR, graphics::Vertex::Attribute::Usage::TEXTURE_COORDINATES0},
                                    {{"color", graphics::DataType::FLOAT_VECTOR4}},
                                    {{"modelViewProj", graphics::DataType::FLOAT_MATRIX4}});

                assetBundle.setShader(SHADER_TEXTURE, textureShader);

                std::shared_ptr<graphics::Shader> colorShader = std::make_shared<graphics::Shader>(*renderer);

                colorShader->init(std::vector<uint8_t>(),
                                  std::vector<uint8_t>(),
                                  {graphics::Vertex::Attribute::Usage::POSITION, graphics::Vertex::Attribute::Usage::COLOR},
                                  {{"color", graphics::DataType::FLOAT_VECTOR4}},
                                  {{"modelViewProj", graphics::DataType::FLOAT_MATRIX4}});

                assetBundle.setShader(SHADER_COLOR, colorShader);
                break;
            }
        }

        std::shared_ptr<graphics::BlendState> noBlendState = std::make_shared<graphics::BlendState>(*renderer);

        noBlendState->init(false,
                           graphics::BlendState::Factor::ONE, graphics::BlendState::Factor::ZERO,
                           graphics::BlendState::Operation::ADD,
                           graphics::BlendState::Factor::ONE, graphics::BlendState::Factor::ZERO,
                           graphics::BlendState::Operation::ADD);

        assetBundle.setBlendState(BLEND_NO_BLEND, noBlendState);

        std::shared_ptr<graphics::BlendState> addBlendState = std::make_shared<graphics::BlendState>(*renderer);

        addBlendState->init(true,
                            graphics::BlendState::Factor::ONE, graphics::BlendState::Factor::ONE,
                            graphics::BlendState::Operation::ADD,
                            graphics::BlendState::Factor::ONE, graphics::BlendState::Factor::ONE,
                            graphics::BlendState::Operation::ADD);

        assetBundle.setBlendState(BLEND_ADD, addBlendState);

        std::shared_ptr<graphics::BlendState> multiplyBlendState = std::make_shared<graphics::BlendState>(*renderer);

        multiplyBlendState->init(true,
                                 graphics::BlendState::Factor::DEST_COLOR, graphics::BlendState::Factor::ZERO,
                                 graphics::BlendState::Operation::ADD,
                                 graphics::BlendState::Factor::ONE, graphics::BlendState::Factor::ONE,
                                 graphics::BlendState::Operation::ADD);

        assetBundle.setBlendState(BLEND_MULTIPLY, multiplyBlendState);

        std::shared_ptr<graphics::BlendState> alphaBlendState = std::make_shared<graphics::BlendState>(*renderer);

        alphaBlendState->init(true,
                              graphics::BlendState::Factor::SRC_ALPHA, graphics::BlendState::Factor::INV_SRC_ALPHA,
                              graphics::BlendState::Operation::ADD,
                              graphics::BlendState::Factor::ONE, graphics::BlendState::Factor::ONE,
                              graphics::BlendState::Operation::ADD);

        assetBundle.setBlendState(BLEND_ALPHA, alphaBlendState);

        std::shared_ptr<graphics::BlendState> screenBlendState = std::make_shared<graphics::BlendState>(*renderer);

        screenBlendState->init(true,
                               graphics::BlendState::Factor::ONE, graphics::BlendState::Factor::INV_SRC_COLOR,
                               graphics::BlendState::Operation::ADD,
                               graphics::BlendState::Factor::ONE, graphics::BlendState::Factor::ONE,
                               graphics::BlendState::Operation::ADD);

        assetBundle.setBlendState(BLEND_SCREEN, screenBlendState);

        std::shared_ptr<graphics::Texture> whitePixelTexture = std::make_shared<graphics::Texture>(*renderer);
        whitePixelTexture->init({255, 255, 255, 255}, Size2(1.0F, 1.0F), 0, 1);
        assetBundle.setTexture(TEXTURE_WHITE_PIXEL, whitePixelTexture);
    }

    void Engine::start()
    {
        if (!active)
        {
            std::unique_ptr<SystemEvent> event(new SystemEvent());
            event->type = Event::Type::ENGINE_START;
            eventDispatcher.postEvent(std::move(event));

            active = true;
            paused = false;

#if OUZEL_MULTITHREADED
            updateThread = std::thread(&Engine::main, this);
#else
            main();
#endif
        }
    }

    void Engine::pause()
    {
        if (active && !paused)
        {
            std::unique_ptr<SystemEvent> event(new SystemEvent());
            event->type = Event::Type::ENGINE_PAUSE;
            eventDispatcher.postEvent(std::move(event));

            paused = true;
        }
    }

    void Engine::resume()
    {
        if (active && paused)
        {
            std::unique_ptr<SystemEvent> event(new SystemEvent());
            event->type = Event::Type::ENGINE_RESUME;
            eventDispatcher.postEvent(std::move(event));

            paused = false;

#if OUZEL_MULTITHREADED
            updateCondition.notify_all();
#endif
        }
    }

    void Engine::exit()
    {
        paused = true;

        if (active)
        {
            std::unique_ptr<SystemEvent> event(new SystemEvent());
            event->type = Event::Type::ENGINE_STOP;
            eventDispatcher.postEvent(std::move(event));

            active = false;
        }

#if OUZEL_MULTITHREADED
        if (updateThread.joinable() &&
            updateThread.get_id() != std::this_thread::get_id())
        {
            std::unique_lock<std::mutex> lock(updateMutex);
            updateCondition.notify_all();
            lock.unlock();
            updateThread.join();
        }
#endif
    }

    void Engine::update()
    {
        eventDispatcher.dispatchEvents();

        std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
        auto diff = currentTime - previousUpdateTime;

        if (diff > std::chrono::milliseconds(1)) // at least one millisecond has passed
        {
            if (diff > std::chrono::seconds(1000 / 20)) diff = std::chrono::milliseconds(1000 / 20); // limit the update rate to a minimum 20 FPS

            previousUpdateTime = currentTime;
            float delta = std::chrono::duration_cast<std::chrono::microseconds>(diff).count() / 1000000.0F;

            std::unique_ptr<UpdateEvent> updateEvent(new UpdateEvent());
            updateEvent->type = Event::Type::UPDATE;
            updateEvent->delta = delta;
            eventDispatcher.dispatchEvent(std::move(updateEvent));
        }

        inputManager->update();
        window->update();
        audio->update();

        if (renderer->getRefillQueue())
            sceneManager.draw();

        if (oneUpdatePerFrame) renderer->waitForNextFrame();
    }

    void Engine::main()
    {
        setCurrentThreadName("Game");

        try
        {
            std::unique_ptr<Application> application = ouzel::main(args);

#if OUZEL_MULTITHREADED
            while (active)
            {
                if (!paused)
                    update();
                else
                {
                    std::unique_lock<std::mutex> lock(updateMutex);
                    while (active && paused)
                        updateCondition.wait(lock);
                }
            }

            eventDispatcher.dispatchEvents();
#endif
        }
        catch (const std::exception& e)
        {
            engine->log(Log::Level::ERR) << e.what();
            exit();
        }
    }

    void Engine::openURL(const std::string&)
    {
    }

    void Engine::setScreenSaverEnabled(bool newScreenSaverEnabled)
    {
        screenSaverEnabled = newScreenSaverEnabled;
    }
}
