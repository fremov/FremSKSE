#include "SKSE/API.h"
#include "InputHandler.h"
#include "HUDManager.h"
#include "Utils.h"
#include "pch.h"
#include "SkillWidget.h"
#include "MenuHandler.h"
#include <keyhandler/keyhandler.h>

// Объявление внешних переменных
PRISMA_UI_API::IVPrismaUI1* PrismaUI = nullptr;
PrismaView view = 0;  // Единственное определение глобальной переменной

class DamageLogManager {
public:
    static void AddDamageEntry(const std::string& attacker, const std::string& target, int damage) {
        if (!PrismaUI || !PrismaUI->IsValid(view)) {
            logger::warn("PrismaUI not available for damage log");
            return;
        }

        std::string script = "addDamageEntry('" +
            EscapeString(attacker) + "','" +
            EscapeString(target) + "'," +
            std::to_string(damage) + ")";

        PrismaUI->Invoke(view, script.c_str());

        logger::info("Damage log entry: {} -> {} ({} damage)", attacker, target, damage);
    }

    static void OpenDamageLog() {
        if (!PrismaUI || !PrismaUI->IsValid(view)) {
            return;
        }

        PrismaUI->Invoke(view, "openDamageLog()");
        logger::info("Damage log opened");
    }

    static void CloseDamageLog() {
        if (!PrismaUI || !PrismaUI->IsValid(view)) {
            return;
        }

        PrismaUI->Invoke(view, "closeDamageLog()");
        logger::info("Damage log closed");
    }

    // Функция для запуска таймера и снятия фокуса
    static void StartTimer() {
        if (!PrismaUI || !PrismaUI->IsValid(view)) {
            return;
        }

        // Закрываем лог и снимаем фокус
        CloseDamageLog();
        PrismaUI->Unfocus(view);

        // Запускаем таймер в React компоненте
        PrismaUI->Invoke(view, "startTimer()");

        logger::info("Timer started, log closed and view unfocused");
    }

private:
    static std::string EscapeString(const std::string& input) {
        std::string result;
        for (char c : input) {
            if (c == '\'') result += "\\'";
            else if (c == '\\') result += "\\\\";
            else result += c;
        }
        return result;
    }
};

class OnWeaponHit
{
public:
    static void Hook()
    {
        _weapon_hit = SKSE::GetTrampoline().write_call<5>(REL::ID(37673).address() + 0x3C0, weapon_hit);
    }

private:
    static void weapon_hit(RE::Actor* target, RE::HitData& hit_data) {
        // Проверяем, что атакующий - игрок, цель жива и удар актуален
        if (hit_data.aggressor && target &&
            !target->IsDead() && // Цель должна быть жива
            hit_data.aggressor.get()->IsPlayerRef()) { // Атакующий должен быть игроком

            std::string attackerName = hit_data.aggressor.get()->GetDisplayFullName();
            std::string targetName = target->GetDisplayFullName();
            int damage = static_cast<int>(hit_data.totalDamage);

            // Отправляем данные в лог урона
            DamageLogManager::AddDamageEntry(attackerName, targetName, damage);
        }
        return _weapon_hit(target, hit_data);
    }

    static inline REL::Relocation<decltype(weapon_hit)> _weapon_hit;
};

static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message) {
    using namespace SKSE;
    using namespace RE;

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
            view = PrismaUI->CreateView("FremUI/index.html", [](PrismaView createdView) {  // Изменил имя параметра
                logger::info("PrismaUI view created successfully");

                // Инициализируем все системы после создания view
                SkillWidget::Initialize();
                SkillWidget::Start();
                SKSE::GetTrampoline().create(228);
                OnWeaponHit::Hook();

                // Регистрируем обработчики событий
                Input::InputEventHandler::Register();
                MenuHandler::register_();
                SkillWidget::Initialize();

                // Регистрируем обработчик клавиши F3 для управления логом урона
                KeyHandler::RegisterSink();
                KeyHandler* keyHandler = KeyHandler::GetSingleton();
                const uint32_t TOGGLE_FOCUS_KEY = 0x3D; // F3 key
                const uint32_t START_TIMER_KEY = 0x3E; // F4 key для запуска таймера

                // Обработчик F3 - переключение фокуса/лога
                auto toggleEventHandler = keyHandler->Register(TOGGLE_FOCUS_KEY, KeyEventType::KEY_DOWN, []() {
                    auto hasFocus = PrismaUI->HasFocus(view);

                    if (!hasFocus) {
                        // Фокусируемся и открываем лог
                        if (PrismaUI->Focus(view)) {
                            DamageLogManager::OpenDamageLog();
                            logger::info("View focused and damage log opened");
                        }
                        else {
                            logger::error("Failed to focus view");
                        }
                    }
                    else {
                        // Снимаем фокус и закрываем лог
                        PrismaUI->Unfocus(view);
                        DamageLogManager::CloseDamageLog();
                        logger::info("View unfocused and damage log closed");
                    }
                    });

                // Обработчик F4 - запуск таймера и снятие фокуса
                auto startTimerHandler = keyHandler->Register(START_TIMER_KEY, KeyEventType::KEY_DOWN, []() {
                    DamageLogManager::StartTimer();
                    });

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