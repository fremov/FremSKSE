#include "SKSE/API.h"
#include "InputHandler.h"
#include "HUDManager.h"
#include "Utils.h"
#include "pch.h"
#include "SkillWidget.h"
#include "MenuHandler.h"

// Объявление внешних переменных
PRISMA_UI_API::IVPrismaUI1* PrismaUI = nullptr;
PrismaView view = 0;  // Единственное определение глобальной переменной

class DamageDisplayManager {
public:
    static void ShowDamageNotification(const std::string& attacker, const std::string& target, int damage) {
        if (!PrismaUI || !PrismaUI->IsValid(view)) {
            logger::warn("PrismaUI not available for damage display");
            return;
        }

        std::string script = "showDamageNotification('" +
            EscapeString(attacker) + "','" +
            EscapeString(target) + "'," +
            std::to_string(damage) + ")";

        PrismaUI->Invoke(view, script.c_str());

        logger::info("Damage notification: {} -> {} ({} damage)", attacker, target, damage);
    }

    static void HideDamageNotification() {
        if (!PrismaUI || !PrismaUI->IsValid(view)) {
            return;
        }

        PrismaUI->Invoke(view, "hideDamageNotification()");
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
        // Проверяем, что цель жива и удар актуален
        if (hit_data.aggressor && target &&
            !target->IsDead() && // Цель должна быть жива
            (target->IsPlayerRef() || hit_data.aggressor.get()->IsPlayerRef())) {

            std::string attackerName = hit_data.aggressor.get()->GetDisplayFullName();
            std::string targetName = target->GetDisplayFullName();
            int damage = static_cast<int>(hit_data.totalDamage);

            // Отправляем только базовые данные
            DamageDisplayManager::ShowDamageNotification(attackerName, targetName, damage);
        }
        return _weapon_hit(target, hit_data);
    }

    static inline REL::Relocation<decltype(weapon_hit)> _weapon_hit;
};



//class OnEquip
//{
//public:
//    static void Hook()
//    {
//        _foo =
//            SKSE::GetTrampoline().write_call<5>(REL::ID(37938).address() + 0xE5,
//                foo);
//    }
//
//private:
//    static void foo(RE::ActorEquipManager* equip_manager,
//        RE::Actor* actor,
//        RE::TESBoundObject* bound_object,
//        void* extra_data_list)
//    {
//
//        if (!equip_manager || !actor || !bound_object || !extra_data_list) {
//            return _foo(equip_manager, actor, bound_object, extra_data_list);
//        }
//        if (bound_object->GetFormType() == RE::FormType::AlchemyItem) {
//            RE::DebugMessageBox("KvazarSOSI");
//            logger::info("effects {}", bound_object->As<RE::AlchemyItem>()->effects[0]);
//            logger::info("effects {}", bound_object->As<RE::AlchemyItem>()->boundData);
//        }
//        return _foo(equip_manager, actor, bound_object, extra_data_list);
//
//    }
//
//    static inline REL::Relocation<decltype(foo)> _foo;
//};



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
            view = PrismaUI->CreateView("FremUI/index.html", [](PrismaView view) {
                logger::info("PrismaUI view created successfully");

                // Инициализируем все системы после создания view
                SkillWidget::Initialize();
                SkillWidget::Start();
                SKSE::GetTrampoline().create(228);
                OnWeaponHit::Hook();
                //OnEquip::Hook();
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