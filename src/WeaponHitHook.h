#pragma once
#include "SKSE/SKSE.h"
#include "DamageLogManager.h"

namespace FremDamageLog {
    class WeaponHitHook {
    public:
        static void Install();

    private:
        static void Hook(RE::Actor* target, RE::HitData& hit_data);
        static inline REL::Relocation<decltype(Hook)> _original_weapon_hit;
    };
}