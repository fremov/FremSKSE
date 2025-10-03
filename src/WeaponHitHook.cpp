#include "WeaponHitHook.h"

namespace FremDamageLog {
    void WeaponHitHook::Install() {
        _original_weapon_hit = SKSE::GetTrampoline().write_call<5>(REL::ID(37673).address() + 0x3C0, Hook);
        logger::info("FremDamageLog weapon hit hook installed");
    }

    void WeaponHitHook::Hook(RE::Actor* target, RE::HitData& hit_data) {
        // ���������, ��� ��������� - �����, ���� ���� � ���� ��������
        if (hit_data.aggressor && target &&
            !target->IsDead() && // ���� ������ ���� ����
            hit_data.aggressor.get()->IsPlayerRef()) { // ��������� ������ ���� �������

            std::string attackerName = hit_data.aggressor.get()->GetDisplayFullName();
            std::string targetName = target->GetDisplayFullName();
            int damage = static_cast<int>(hit_data.totalDamage);

            // ���������� ������ � ��� �����
            DamageLogManager::AddDamageEntry(attackerName, targetName, damage);
        }
        return _original_weapon_hit(target, hit_data);
    }
}