// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_GRAPHICS_RENDERDEVICED3D11_HPP
#define OUZEL_GRAPHICS_RENDERDEVICED3D11_HPP

#include "core/Setup.h"

#if OUZEL_COMPILE_DIRECT3D11

#include <atomic>
#include <map>
#include <memory>
#include <thread>
#include <vector>
#include <d3d11.h>
#include "graphics/RenderDevice.hpp"
#include "graphics/direct3d11/TextureResourceD3D11.hpp"

namespace ouzel
{
    namespace graphics
    {
        class RenderDeviceD3D11 final: public RenderDevice
        {
            friend Renderer;
        public:
            ~RenderDeviceD3D11();

            std::vector<Size2> getSupportedResolutions() const override;

            ID3D11Device* getDevice() const { return device; }
            ID3D11DeviceContext* getContext() const { return context; }

            ID3D11SamplerState* getSamplerState(const SamplerStateDesc& desc);
            void setFullscreen(bool newFullscreen);

        protected:
            RenderDeviceD3D11(const std::function<void(const Event&)>& initCallback);

            void init(Window* newWindow,
                      const Size2& newSize,
                      uint32_t newSampleCount,
                      Texture::Filter newTextureFilter,
                      uint32_t newMaxAnisotropy,
                      bool newVerticalSync,
                      bool newDepth,
                      bool newDebugRenderer) override;

            virtual void setSize(const Size2& newSize);

            void setClearColorBuffer(bool clear);
            void setClearDepthBuffer(bool clear);
            void setClearColor(Color newClearColor);
            void setClearDepth(float newClearDepth);

            void process() override;
            void resizeBackBuffer(UINT newWidth, UINT newHeight);
            void uploadBuffer(ID3D11Buffer* buffer, const void* data, uint32_t dataSize);
            void generateScreenshot(const std::string& filename) override;
            void main();

            IDXGIOutput* getOutput() const;

            ID3D11Device* device = nullptr;
            ID3D11DeviceContext* context = nullptr;
            IDXGISwapChain* swapChain = nullptr;
            IDXGIAdapter* adapter = nullptr;
            ID3D11Texture2D* backBuffer = nullptr;
            ID3D11RenderTargetView* renderTargetView = nullptr;
            std::map<SamplerStateDesc, ID3D11SamplerState*> samplerStates;
            ID3D11RasterizerState* rasterizerStates[12];
            ID3D11Texture2D* depthStencilTexture = nullptr;
            ID3D11DepthStencilView* depthStencilView = nullptr;
            ID3D11DepthStencilState* defaultDepthStencilState = nullptr;

            UINT frameBufferWidth = 0;
            UINT frameBufferHeight = 0;

            UINT swapInterval = 0;
            FLOAT frameBufferClearColor[4];

            std::atomic_bool running;
            std::thread renderThread;

            std::vector<std::unique_ptr<RenderResourceD3D11>> resources;
        };
    } // namespace graphics
} // namespace ouzel

#endif

#endif // OUZEL_GRAPHICS_RENDERDEVICED3D11_HPP
