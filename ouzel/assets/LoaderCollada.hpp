// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_ASSETS_LOADERCOLLADA_HPP
#define OUZEL_ASSETS_LOADERCOLLADA_HPP

#include "assets/Loader.hpp"

namespace ouzel
{
    namespace assets
    {
        class LoaderCollada final: public Loader
        {
        public:
            static const uint32_t TYPE = Loader::SKINNED_MESH;

            explicit LoaderCollada(Cache& initCache);
            bool loadAsset(Bundle& bundle, const std::string& filename, const std::vector<uint8_t>& data, bool mipmaps = true) override;
        };
    } // namespace assets
} // namespace ouzel

#endif // OUZEL_ASSETS_LOADERCOLLADA_HPP
