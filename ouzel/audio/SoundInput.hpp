// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_AUDIO_SOUNDINPUT_HPP
#define OUZEL_AUDIO_SOUNDINPUT_HPP

#include "audio/AudioDevice.hpp"

namespace ouzel
{
    namespace audio
    {
        class SoundOutput;

        class SoundInput
        {
            friend SoundOutput;
        public:
            virtual ~SoundInput();

            inline SoundOutput* getOutput() const { return output; }
            void setOutput(SoundOutput* newOutput);

        protected:
            SoundOutput* output = nullptr;
        };
    } // namespace audio
} // namespace ouzel

#endif // OUZEL_AUDIO_SOUNDINPUT_HPP
