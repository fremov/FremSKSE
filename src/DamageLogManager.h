#pragma once
#include "SKSE/SKSE.h"
#include "pch.h"

namespace FremDamageLog {
    class DamageLogManager {
    public:
        static void Initialize(PRISMA_UI_API::IVPrismaUI1* prismaUI, PrismaView prismaView);
        static void AddDamageEntry(const std::string& attacker, const std::string& target, int damage);
        static void OpenDamageLog();
        static void CloseDamageLog();
        static void StartTimer();
        static void RegisterKeybinds();

    private:
        static std::string EscapeString(const std::string& input);
        static inline PRISMA_UI_API::IVPrismaUI1* PrismaUI = nullptr;
        static inline PrismaView View = 0;
    };
}