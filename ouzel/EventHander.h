// Copyright (C) 2016 Elviss Strazdins
// This file is part of the Ouzel engine.

#pragma once

#include <functional>
#include "Event.h"

namespace ouzel
{
    class EventDispatcher;

    class EventHandler
    {
        friend EventDispatcher;

    public:
        static const int32_t PRIORITY_MAX = 0x1000;

        EventHandler(int32_t pPriority = 0): priority(pPriority) { }

        std::function<bool(const KeyboardEventPtr&, const VoidPtr&)> keyboardHandler;
        std::function<bool(const MouseEventPtr&, const VoidPtr&)> mouseHandler;
        std::function<bool(const TouchEventPtr&, const VoidPtr&)> touchHandler;
        std::function<bool(const GamepadEventPtr&, const VoidPtr&)> gamepadHandler;
        std::function<bool(const WindowEventPtr&, const VoidPtr&)> windowHandler;
        std::function<bool(const SystemEventPtr&, const VoidPtr&)> systemHandler;
        std::function<bool(const UIEventPtr&, const VoidPtr&)> uiHandler;

    protected:
        int32_t priority;
        bool remove = false;
    };
}
