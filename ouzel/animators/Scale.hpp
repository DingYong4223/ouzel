// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_SCALE_HPP
#define OUZEL_SCALE_HPP

#include "animators/Animator.hpp"
#include "math/Vector3.hpp"

namespace ouzel
{
    namespace scene
    {
        class Scale final: public Animator
        {
        public:
            Scale(float initLength, const Vector3& initScale, bool initRelative = false);

            void play() override;

        protected:
            void updateProgress() override;

        private:
            Vector3 scale;
            Vector3 startScale;
            Vector3 targetScale;
            Vector3 diff;
            bool relative;
        };
    } // namespace scene
} // namespace ouzel

#endif // OUZEL_SCALE_HPP
