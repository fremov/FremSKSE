#include "DamageLogManager.h"
#include <keyhandler/keyhandler.h>

namespace FremDamageLog {
    void DamageLogManager::Initialize(PRISMA_UI_API::IVPrismaUI1* prismaUI, PrismaView prismaView) {
        PrismaUI = prismaUI;
        View = prismaView;
        logger::info("FremDamageLog initialized");
    }

    void DamageLogManager::AddDamageEntry(const std::string& attacker, const std::string& target, int damage) {
        if (!PrismaUI || !PrismaUI->IsValid(View)) {
            logger::warn("PrismaUI not available for damage log");
            return;
        }

        std::string script = "fremAddDamageEntry('" +
            EscapeString(attacker) + "','" +
            EscapeString(target) + "'," +
            std::to_string(damage) + ")";

        PrismaUI->Invoke(View, script.c_str());
        logger::info("FremDamageLog entry: {} -> {} ({} damage)", attacker, target, damage);
    }

    void DamageLogManager::OpenDamageLog() {
        if (!PrismaUI || !PrismaUI->IsValid(View)) {
            return;
        }

        PrismaUI->Invoke(View, "fremOpenDamageLog()");
        logger::info("FremDamageLog opened");
    }

    void DamageLogManager::CloseDamageLog() {
        if (!PrismaUI || !PrismaUI->IsValid(View)) {
            return;
        }

        PrismaUI->Invoke(View, "fremCloseDamageLog()");
        logger::info("FremDamageLog closed");
    }

    void DamageLogManager::StartTimer() {
        if (!PrismaUI || !PrismaUI->IsValid(View)) {
            return;
        }

        // Закрываем лог и снимаем фокус
        CloseDamageLog();
        PrismaUI->Unfocus(View);

        // Запускаем таймер в React компоненте
        PrismaUI->Invoke(View, "fremStartTimer()");

        logger::info("FremDamageLog timer started, log closed and view unfocused");
    }

    void DamageLogManager::RegisterKeybinds() {
        KeyHandler::RegisterSink();
        KeyHandler* keyHandler = KeyHandler::GetSingleton();

        const uint32_t TOGGLE_FOCUS_KEY = 0x3D; // F3 key
        const uint32_t START_TIMER_KEY = 0x3E; // F4 key

        // Обработчик F3 - переключение фокуса/лога
        keyHandler->Register(TOGGLE_FOCUS_KEY, KeyEventType::KEY_DOWN, []() {
            auto hasFocus = PrismaUI->HasFocus(View);

            if (!hasFocus) {
                // Фокусируемся и открываем лог
                if (PrismaUI->Focus(View)) {
                    OpenDamageLog();
                    logger::info("FremDamageLog view focused and damage log opened");
                }
                else {
                    logger::error("Failed to focus FremDamageLog view");
                }
            }
            else {
                // Снимаем фокус и закрываем лог
                PrismaUI->Unfocus(View);
                CloseDamageLog();
                logger::info("FremDamageLog view unfocused and damage log closed");
            }
            });

        // Обработчик F4 - запуск таймера и снятие фокуса
        keyHandler->Register(START_TIMER_KEY, KeyEventType::KEY_DOWN, []() {
            StartTimer();
            });

        logger::info("FremDamageLog keybinds registered (F3: Toggle, F4: Start Timer)");
    }

    std::string DamageLogManager::EscapeString(const std::string& input) {
        std::string result;
        for (char c : input) {
            if (c == '\'') result += "\\'";
            else if (c == '\\') result += "\\\\";
            else result += c;
        }
        return result;
    }
}