#include "HUDManager.h"
#include "Utils.h"

namespace HUDManager {
    std::atomic<bool> g_running = true;

    void UpdateCustomBars() {
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

        auto regHp = 0.f;
        auto regMp = 0.f;
        auto regSt = 0.f;

        // �������� ����������������� ���� �� ���������� ����������
        auto data = RE::TESDataHandler::GetSingleton();
        auto ManaLock = data->LookupForm<RE::TESGlobal>(0x0E9BB1, "STB.esp");
        float reservedMana = 0.f;

        if (ManaLock) {
            reservedMana = ManaLock->value;
        }

        for (auto active_effect : *player->AsMagicTarget()->GetActiveEffectList()) {
            if (active_effect->effect->baseEffect->data.primaryAV == RE::ActorValue::kHealth &&
                !active_effect->flags.any(RE::ActiveEffect::Flag::kInactive) &&
                active_effect->effect->baseEffect->HasKeywordString("aaMZkw_RestoreHealthRace")) {
                regHp += active_effect->magnitude;
            }
            else if (active_effect->effect->baseEffect->data.primaryAV == RE::ActorValue::kMagicka &&
                !active_effect->flags.any(RE::ActiveEffect::Flag::kInactive) &&
                active_effect->effect->baseEffect->HasKeywordString("aaMZkw_RestoreMagickaRace")) {
                regMp += active_effect->magnitude;
            }
            else if (active_effect->effect->baseEffect->data.primaryAV == RE::ActorValue::kStamina &&
                !active_effect->flags.any(RE::ActiveEffect::Flag::kInactive) &&
                active_effect->effect->baseEffect->HasKeywordString("aaMZkw_RestoreStaminaRace")) {
                regSt += active_effect->magnitude;
            }
        }

        std::string script = "updateStats(" +
            std::to_string(hp) + "," +
            std::to_string(mp) + "," +
            std::to_string(st) + "," +
            std::to_string(maxHp) + "," +
            std::to_string(maxMp) + "," +
            std::to_string(maxSt) + "," +
            Utils::format_float(regHp, 2) + "," +
            Utils::format_float(regMp, 2) + "," +
            Utils::format_float(regSt, 2) + "," +
            Utils::format_float(reservedMana, 2) + "," + // ��������� ����������������� ����
            "false)";

        if (PrismaUI->IsValid(view)) {
            PrismaUI->Invoke(view, script.c_str());
            logger::info("Sent stats update: {}", script);
        }
    }
    void UpdateThread() {
        while (g_running) {
            UpdateCustomBars();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    }
}