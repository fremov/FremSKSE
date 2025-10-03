#include "SKSE/API.h"
#include "InputHandler.h"
#include "HUDManager.h"
#include "Utils.h"
#include "SkillWidget.h"
#include "MenuHandler.h"
#include "DamageLogManager.h"
#include "WeaponHitHook.h"
#include "pch.h"


// Объявление внешних переменных
PRISMA_UI_API::IVPrismaUI1* PrismaUI = nullptr;
PrismaView view = 0;

static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message) {
    using namespace SKSE;
    using namespace RE;
    using namespace FremDamageLog;

    switch (message->type) {
    case MessagingInterface::kPostLoad:
        // Инициализируем PrismaUI API
        PrismaUI = static_cast<PRISMA_UI_API::IVPrismaUI1*>(
            PRISMA_UI_API::RequestPluginAPI(PRISMA_UI_API::InterfaceVersion::V1)
            );

        if (!PrismaUI) {
            logger::error("Failed to get PrismaUI API");
            return;
        }
        logger::info("PrismaUI API successfully initialized");
        break;

    case MessagingInterface::kDataLoaded:
        // Создаем PrismaUI view
        if (PrismaUI) {
            view = PrismaUI->CreateView("FremUI/index.html", [](PrismaView createdView) {
                logger::info("PrismaUI view created successfully");

                // Инициализируем все системы после создания view
                SkillWidget::Initialize();
                SkillWidget::Start();
                SKSE::GetTrampoline().create(228);

                // Инициализируем систему лога урона
                DamageLogManager::Initialize(PrismaUI, view);
                WeaponHitHook::Install();
                DamageLogManager::RegisterKeybinds();

                // Регистрируем обработчики событий
                Input::InputEventHandler::Register();
                MenuHandler::register_();
                SkillWidget::Initialize();

                // Запускаем поток обновления HUD
                HUDManager::g_running = true;
                std::thread(HUDManager::UpdateThread).detach();

                logger::info("All systems initialized successfully");
                });

            if (view == 0) {
                logger::error("Failed to create PrismaUI view");
            }
        }
        break;
    }
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse) {
    SKSE::Init(a_skse);

    auto messaging = SKSE::GetMessagingInterface();
    if (messaging) {
        messaging->RegisterListener("SKSE", SKSEMessageHandler);
    }
    else {
        logger::error("Failed to get messaging interface");
        return false;
    }

    return true;
}