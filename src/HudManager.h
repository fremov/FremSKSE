#pragma once
#include "RE/Skyrim.h"
#include "PrismaUI_API.h"
#include <thread>
#include <atomic>
#include "RE/A/ActorValues.h"

extern PRISMA_UI_API::IVPrismaUI1* PrismaUI;
extern PrismaView view;  // Только объявление


namespace HUDManager {
    void UpdateCustomBars();
    void UpdateThread();
    float GetExperienceForLevel(int level);
    int CheckResist(RE::ActorValue av, RE::Actor* a);
    extern std::atomic<bool> g_running;
}