#pragma once

#include "RE/Skyrim.h"
#include "PrismaUI_API.h"

extern PRISMA_UI_API::IVPrismaUI1* PrismaUI;
extern PrismaView view;  // Только объявление

namespace Input {
    enum : uint32_t {
        kInvalid = static_cast<uint32_t>(-1),
        kKeyboardOffset = 0,
        kMouseOffset = 256,
        kGamepadOffset = 266
    };

    class InputEventHandler : public RE::BSTEventSink<RE::InputEvent*> {
    public:
        static void Register();
    private:
        static InputEventHandler* GetSingleton();
        using EventResult = RE::BSEventNotifyControl;
        std::uint32_t GetGamepadIndex(RE::BSWin32GamepadDevice::Key a_key);
        std::uint32_t GetKey(RE::BSFixedString _event);
        EventResult ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*);
    };
}