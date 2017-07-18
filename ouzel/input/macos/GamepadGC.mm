// Copyright (C) 2017 Elviss Strazdins
// This file is part of the Ouzel engine.

#include <objc/message.h>
#include "GamepadGC.h"
#include "core/Engine.h"
#include "core/CompileConfig.h"
#include "events/EventDispatcher.h"
#include "utils/Log.h"

namespace ouzel
{
    namespace input
    {
        GamepadGC::GamepadGC(GCControllerPtr aController):
            controller(aController)
        {
            if (controller.extendedGamepad)
            {
                // left thumbstick
                controller.extendedGamepad.leftThumbstick.up.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::LEFT_THUMB_UP, pressed, value);
                };
                controller.extendedGamepad.leftThumbstick.down.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::LEFT_THUMB_DOWN, pressed, value);
                };
                controller.extendedGamepad.leftThumbstick.left.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::LEFT_THUMB_LEFT, pressed, value);
                };
                controller.extendedGamepad.leftThumbstick.right.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::LEFT_THUMB_RIGHT, pressed, value);
                };

                // right thumbstick
                controller.extendedGamepad.rightThumbstick.up.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::RIGHT_THUMB_UP, pressed, value);
                };
                controller.extendedGamepad.rightThumbstick.down.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::RIGHT_THUMB_DOWN, pressed, value);
                };
                controller.extendedGamepad.rightThumbstick.left.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::RIGHT_THUMB_LEFT, pressed, value);
                };
                controller.extendedGamepad.rightThumbstick.right.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::RIGHT_THUMB_RIGHT, pressed, value);
                };

                // D-pad
                controller.extendedGamepad.dpad.up.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::DPAD_UP, pressed, value);
                };
                controller.extendedGamepad.dpad.down.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::DPAD_DOWN, pressed, value);
                };
                controller.extendedGamepad.dpad.left.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::DPAD_LEFT, pressed, value);
                };
                controller.extendedGamepad.dpad.right.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::DPAD_RIGHT, pressed, value);
                };

                // buttons
                controller.extendedGamepad.buttonA.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::FACE1, pressed, value);
                };
                controller.extendedGamepad.buttonB.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::FACE2, pressed, value);
                };
                controller.extendedGamepad.buttonX.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::FACE3, pressed, value);
                };
                controller.extendedGamepad.buttonY.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::FACE4, pressed, value);
                };

                // triggers and shoulders
                controller.extendedGamepad.leftShoulder.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::LEFT_SHOULDER, pressed, value);
                };
                controller.extendedGamepad.leftTrigger.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::LEFT_TRIGGER, pressed, value);
                };
                controller.extendedGamepad.rightShoulder.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::RIGHT_SHOULDER, pressed, value);
                };
                controller.extendedGamepad.rightTrigger.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::RIGHT_TRIGGER, pressed, value);
                };

            }
            else if (controller.gamepad)
            {
                // D-pad
                controller.gamepad.dpad.up.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::DPAD_UP, pressed, value);
                };
                controller.gamepad.dpad.down.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::DPAD_DOWN, pressed, value);
                };
                controller.gamepad.dpad.left.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::DPAD_LEFT, pressed, value);
                };
                controller.gamepad.dpad.right.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::DPAD_RIGHT, pressed, value);
                };

                // buttons
                controller.gamepad.buttonA.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::FACE1, pressed, value);
                };
                controller.gamepad.buttonB.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::FACE2, pressed, value);
                };
                controller.gamepad.buttonX.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::FACE3, pressed, value);
                };
                controller.gamepad.buttonY.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::FACE4, pressed, value);
                };
            }
            else if (controller.microGamepad)
            {
                // D-pad
                controller.microGamepad.dpad.up.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::DPAD_UP, pressed, value);
                };
                controller.microGamepad.dpad.down.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::DPAD_DOWN, pressed, value);
                };
                controller.microGamepad.dpad.left.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::DPAD_LEFT, pressed, value);
                };
                controller.microGamepad.dpad.right.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::DPAD_RIGHT, pressed, value);
                };

                // buttons
                controller.microGamepad.buttonA.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::FACE1, pressed, value);
                };
                controller.microGamepad.buttonX.valueChangedHandler = ^(GCControllerButtonInput*, float value, BOOL pressed) {
                    handleButtonValueChange(GamepadButton::FACE3, pressed, value);
                };
            }

            controller.controllerPausedHandler = ^(GCController*) {
                handleButtonValueChange(GamepadButton::PAUSE, true, 1.0f);
                handleButtonValueChange(GamepadButton::PAUSE, false, 0.0f);
            };

            if (controller.vendorName)
            {
                name = [controller.vendorName cStringUsingEncoding:NSASCIIStringEncoding];
            }
            
            attached = (controller.isAttachedToDevice == YES);
        }

        void GamepadGC::setAbsoluteDpadValues(bool absoluteDpadValues)
        {
            controller.microGamepad.reportsAbsoluteDpadValues = absoluteDpadValues ? YES : NO;
        }

        bool GamepadGC::isAbsoluteDpadValues() const
        {
            return controller.microGamepad.reportsAbsoluteDpadValues == YES;
        }

        int32_t GamepadGC::getPlayerIndex() const
        {
            return static_cast<int32_t>(controller.playerIndex);
        }

        bool GamepadGC::setPlayerIndex(int32_t playerIndex)
        {
            switch (playerIndex)
            {
                case 0:
                    controller.playerIndex = GCControllerPlayerIndex1;
                    break;
                case 1:
                    controller.playerIndex = GCControllerPlayerIndex2;
                    break;
                case 2:
                    controller.playerIndex = GCControllerPlayerIndex3;
                    break;
                case 3:
                    controller.playerIndex = GCControllerPlayerIndex4;
                    break;
                default:
                    controller.playerIndex = GCControllerPlayerIndexUnset;
                    break;
            }

            return true;
        }
    } // namespace input
} // namespace ouzel
