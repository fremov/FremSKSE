#include "InputHandler.h"

namespace Input {
    void InputEventHandler::Register() {
        auto deviceManager = RE::BSInputDeviceManager::GetSingleton();
        deviceManager->AddEventSink(InputEventHandler::GetSingleton());
    }

    InputEventHandler* InputEventHandler::GetSingleton() {
        static InputEventHandler singleton;
        return std::addressof(singleton);
    }

    std::uint32_t InputEventHandler::GetGamepadIndex(RE::BSWin32GamepadDevice::Key a_key) {
        using Key = RE::BSWin32GamepadDevice::Key;

        std::uint32_t index;
        switch (a_key) {
        case Key::kUp:
            index = 0;
            break;
        case Key::kDown:
            index = 1;
            break;
        case Key::kLeft:
            index = 2;
            break;
        case Key::kRight:
            index = 3;
            break;
        case Key::kStart:
            index = 4;
            break;
        case Key::kBack:
            index = 5;
            break;
        case Key::kLeftThumb:
            index = 6;
            break;
        case Key::kRightThumb:
            index = 7;
            break;
        case Key::kLeftShoulder:
            index = 8;
            break;
        case Key::kRightShoulder:
            index = 9;
            break;
        case Key::kA:
            index = 10;
            break;
        case Key::kB:
            index = 11;
            break;
        case Key::kX:
            index = 12;
            break;
        case Key::kY:
            index = 13;
            break;
        case Key::kLeftTrigger:
            index = 14;
            break;
        case Key::kRightTrigger:
            index = 15;
            break;
        default:
            index = kInvalid;
            break;
        }

        return index != kInvalid ? index + kGamepadOffset : kInvalid;
    }

    std::uint32_t InputEventHandler::GetKey(RE::BSFixedString _event) {
        const auto controlMap = RE::ControlMap::GetSingleton();
        auto key = controlMap->GetMappedKey(_event, RE::INPUT_DEVICE::kKeyboard);
        key += kKeyboardOffset;
        return key;
    }

    InputEventHandler::EventResult InputEventHandler::ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) {
        using EventType = RE::INPUT_EVENT_TYPE;
        using DeviceType = RE::INPUT_DEVICE;

        if (!a_event)
            return EventResult::kContinue;

        if (const auto ui = RE::UI::GetSingleton()) {
            for (auto event = *a_event; event; event = event->next) {
                if (event->eventType != EventType::kButton)
                    continue;
                const auto button = static_cast<RE::ButtonEvent*>(event);
                if (!button || (!button->IsPressed() && !button->IsUp()))
                    continue;
                auto key = button->GetIDCode();
                switch (button->device.get()) {
                case DeviceType::kMouse:
                    key += kMouseOffset;
                    break;
                case DeviceType::kKeyboard:
                    key += kKeyboardOffset;
                    break;
                case DeviceType::kGamepad:
                    key = GetGamepadIndex((RE::BSWin32GamepadDevice::Key)key);
                    break;
                default:
                    continue;
                }
                if (key == 0x1C && button->IsDown()) {
                    PrismaUI->Invoke(view, "toggleHintsContainer()");
                }
            }
        }

        return EventResult::kContinue;
    }
}