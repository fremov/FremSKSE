#pragma once
#include "RE/Skyrim.h"
#include "PrismaUI_API.h"
#include <thread>
#include <atomic>


extern PRISMA_UI_API::IVPrismaUI1* PrismaUI;
extern PrismaView view;  // Только объявление


namespace HUDManager {
    void UpdateCustomBars();
    void UpdateThread();
    extern std::atomic<bool> g_running;
}