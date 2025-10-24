#include "HUDManager.h"
#include "Utils.h"
#include "ExperienceWidget.h"
using namespace RE;

namespace HUDManager {
    std::atomic<bool> g_running = true;

    float GetExperienceForLevel(int level) {
        auto fXPLevelUpMult = 0.f;
        auto fXPLevelUpBase = 0.f;
        const auto settings = RE::GameSettingCollection::GetSingleton();

        if (settings) {
            if (auto levelUpBase = settings->GetSetting("fXPLevelUpBase"))
                fXPLevelUpBase = levelUpBase->GetFloat();
            if (auto levelUpMult = settings->GetSetting("fXPLevelUpMult"))
                fXPLevelUpMult = levelUpMult->GetFloat();
        }

        return fXPLevelUpBase + fXPLevelUpMult * level;
    }

    int CheckResist(RE::ActorValue av, RE::Actor* a) {
        static auto resists = []() -> RE::SpellItem* {
            if (auto data = RE::TESDataHandler::GetSingleton())
                return data->LookupForm<RE::SpellItem>(0x0192B9, "STB.esp");
            return nullptr;
            }();

        static auto resistsPerk = []() -> RE::BGSPerk* {
            if (auto data = RE::TESDataHandler::GetSingleton())
                return data->LookupForm<RE::BGSPerk>(0x019388, "STB.esp");
            return nullptr;
            }();

        double DamageResist = pow(0.9f, (a->AsActorValueOwner()->GetActorValue(av) / 100.f));

        if (resists && resistsPerk &&
            a->HasPerk(resistsPerk) &&
            DamageResist <= (1.f - (float)resists->effects[0]->effectItem.duration / 100.f))
        {
            DamageResist = 1.f - (float)resists->effects[0]->effectItem.duration / 100.f;
        }

        auto res = (int)((1 - (float)(DamageResist)) * 100);

        if (resistsPerk && a->HasPerk(resistsPerk) && res > a->GetLevel() + 30)
            res = a->GetLevel() + 30;

        return res;
    }
    bool PlayerInCombat()
    {
        auto player = RE::PlayerCharacter::GetSingleton();
        return player && player->IsInCombat();
    }

    void UpdateCustomBars() {
        static auto manaLock = []() -> RE::TESGlobal* {
            if (auto data = RE::TESDataHandler::GetSingleton())
                return data->LookupForm<RE::TESGlobal>(0x0E9BB1, "STB.esp");
            return nullptr;
            }();

        static auto currentExp = []() -> RE::TESGlobal* {
            if (auto data = RE::TESDataHandler::GetSingleton())
                return data->LookupForm<RE::TESGlobal>(0x015BA5, "STB.esp");
            return nullptr;
            }();

        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player || !PrismaUI || !PrismaUI->IsValid(view)) {
            logger::warn("Cannot update bars: player={}, PrismaUI={}, viewValid={}",
                player != nullptr, PrismaUI != nullptr,
                (PrismaUI && PrismaUI->IsValid(view)));
            return;
        }

        int hp = static_cast<int>(player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth));
        int mp = static_cast<int>(player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kMagicka));
        int st = static_cast<int>(player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina));

        int maxHp = static_cast<int>(Utils::get_total_av(player, RE::ActorValue::kHealth));
        int maxMp = static_cast<int>(Utils::get_total_av(player, RE::ActorValue::kMagicka));
        int maxSt = static_cast<int>(Utils::get_total_av(player, RE::ActorValue::kStamina));

        float regHp = 0.f;
        float regMp = 0.f;
        float regSt = 0.f;
        float reservedMana = 0.f;

        if (manaLock) {
            reservedMana = manaLock->value;
        }

        auto currentLvl = player->GetLevel();
        auto nextLvlExp = GetExperienceForLevel(currentLvl);

        auto fireRes = CheckResist(RE::ActorValue::kResistFire, player);
        auto frostRes = CheckResist(RE::ActorValue::kResistFrost, player);
        auto shockRes = CheckResist(RE::ActorValue::kResistShock, player);
        auto chaosRes = CheckResist(RE::ActorValue::kPoisonResist, player);
        auto damageRes = CheckResist(RE::ActorValue::kDamageResist, player);

        /*logger::info("FireRes {}", fireRes);
        logger::info("frosteRes {}", frostRes);
        logger::info("shockRes {}", shockRes);
        logger::info("chaosRes {}", chaosRes);
        logger::info("damageRes {}", damageRes);*/

        auto intox = TESForm::LookupByEditorID<TESGlobal>("aaMZgv_Potion_Intoxication");
        auto intoxLock = TESForm::LookupByEditorID<TESGlobal>("aaMZgv_Potion_IntoxicationLocked");

        int currentIntox = intox->value;
        int lockedIntox = intoxLock->value;
        int maxIntox = 999;
        /*logger::info("Intox {}", intox->value);
        logger::info("IntoxLock {}", intoxLock->value);*/

        auto playerSpeed = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kSpeedMult);
        //logger::info("speed: {}", playerSpeed);

        auto playerInCombat = PlayerInCombat();
        //logger::info("combat: {}", playerInCombat);

        for (auto active_effect : *player->AsMagicTarget()->GetActiveEffectList()) {
            if (!active_effect || active_effect->flags.any(RE::ActiveEffect::Flag::kInactive))
                continue;

            auto baseEffect = active_effect->effect->baseEffect;
            if (!baseEffect) continue;

            if (baseEffect->data.primaryAV == RE::ActorValue::kHealth &&
                baseEffect->HasKeywordString("aaMZkw_RestoreHealthRace")) {
                regHp += active_effect->magnitude;
            }
            else if (baseEffect->data.primaryAV == RE::ActorValue::kMagicka &&
                baseEffect->HasKeywordString("aaMZkw_RestoreMagickaRace")) {
                regMp += active_effect->magnitude;
            }
            else if (baseEffect->data.primaryAV == RE::ActorValue::kStamina &&
                baseEffect->HasKeywordString("aaMZkw_RestoreStaminaRace")) {
                regSt += active_effect->magnitude;
            }
        }

        std::string script = "updateStatsBars(" +
            std::to_string(hp) + "," +
            std::to_string(mp) + "," +
            std::to_string(st) + "," +
            std::to_string(maxHp) + "," +
            std::to_string(maxMp) + "," +
            std::to_string(maxSt) + "," +
            Utils::format_float(regHp, 2) + "," +
            Utils::format_float(regMp, 2) + "," +
            Utils::format_float(regSt, 2) + "," +
            Utils::format_float(reservedMana, 2) + "," +
            "false)";

        std::string loadingScript = "updateLoadingExperienceData(" +
            std::to_string(currentLvl) + "," +
            std::to_string(currentExp ? currentExp->value : 0) + "," +
            std::to_string(nextLvlExp) + ")";

        std::string resistancesScript = "updateResistancesData(["
            "{type:'fire', value:" + std::to_string(fireRes) + "},"
            "{type:'frost', value:" + std::to_string(frostRes) + "},"
            "{type:'shock', value:" + std::to_string(shockRes) + "},"
            "{type:'chaos', value:" + std::to_string(chaosRes) + "},"
            "{type:'physical', value:" + std::to_string(damageRes) + "},"
            "{type:'speed', value:" + std::to_string(playerSpeed) + "}"
            "])";

        std::string intoxScript = "updateIntoxicationData(" +
            std::to_string(currentIntox) + "," +
            std::to_string(lockedIntox) + "," +
            std::to_string(maxIntox) + ")";

        if (PrismaUI->IsValid(view)) {
            //PrismaUI->Invoke(view, script.c_str());
            /*PrismaUI->Invoke(view, circularScript.c_str());
            logger::info("lvl script: {}", circularScript);*/
            /*PrismaUI->Invoke(view, loadingScript.c_str());
            PrismaUI->Invoke(view, resistancesScript.c_str());
            PrismaUI->Invoke(view, intoxScript.c_str());*/
        }
    }

    void UpdateThread() {
        while (g_running) {
            UpdateCustomBars();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }
}