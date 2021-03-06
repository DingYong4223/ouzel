// Copyright 2015-2018 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_CORE_TIMER_HPP
#define OUZEL_CORE_TIMER_HPP

namespace ouzel
{
    class Timer final
    {
    public:
        Timer();
        ~Timer() = default;

        Timer(const Timer&) = delete;
        Timer& operator=(const Timer&) = delete;

        Timer(Timer&&) = delete;
        Timer& operator=(Timer&&) = delete;

        void update(float delta);
    };
}

#endif // OUZEL_CORE_TIMER_HPP
