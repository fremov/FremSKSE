#pragma once

#include "RE/Skyrim.h"
#include "PrismaUI_API.h"

extern PRISMA_UI_API::IVPrismaUI1* PrismaUI;
extern PrismaView view;
extern bool g_shouldShowHints;  // ���������� ���������� ����������

struct MenuHandler final : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
public:
    [[nodiscard]] static auto get_singleton() noexcept -> MenuHandler*;
    static auto register_() -> void;

    auto ProcessEvent(const RE::MenuOpenCloseEvent* event,
        RE::BSTEventSource<RE::MenuOpenCloseEvent>* event_source)
        -> RE::BSEventNotifyControl override;
};