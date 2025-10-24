#include "ExperienceWidget.h"
#include <cmath>

using namespace RE;

float ExperienceManager::GetExperienceForLevel(int level) {
    float fXPLevelUpMult = 0.f;
    float fXPLevelUpBase = 0.f;

    const auto settings = GameSettingCollection::GetSingleton();
    if (settings) {
        if (auto levelUpBase = settings->GetSetting("fXPLevelUpBase")) {
            fXPLevelUpBase = levelUpBase->GetFloat();
        }
        if (auto levelUpMult = settings->GetSetting("fXPLevelUpMult")) {
            fXPLevelUpMult = levelUpMult->GetFloat();
        }
    }

    return fXPLevelUpBase + fXPLevelUpMult * (level - 1);
}

bool ExperienceManager::IsPlayerInCombat() {
    auto player = PlayerCharacter::GetSingleton();
    return player && player->IsInCombat();
}

bool ExperienceManager::GetUpdatedExperienceData(ExperienceData& outData) {
    auto player = PlayerCharacter::GetSingleton();
    if (!player) return false;

    // Получаем текущий глобал опыта из STB.esp
    static auto currentExpGlobal = []() -> TESGlobal* {
        if (auto dataHandler = TESDataHandler::GetSingleton()) {
            return dataHandler->LookupForm<TESGlobal>(0x015BA5, "STB.esp");
        }
        return nullptr;
        }();

    int newLevel = player->GetLevel();
    float newExp = currentExpGlobal ? currentExpGlobal->value : 0.0f;
    float newNextLevelExp = GetExperienceForLevel(newLevel);
    bool newPlayerInCombat = IsPlayerInCombat();

    // Проверяем повышение уровня - для первого вызова всегда false
    bool levelIncreased = !firstCall && (newLevel > previousData.currentLevel);

    // Сравниваем с предыдущими данными
    bool dataChanged = firstCall ||
        (newLevel != previousData.currentLevel) ||
        (std::abs(newExp - previousData.currentExp) > 0.1f) ||
        (std::abs(newNextLevelExp - previousData.nextLevelExp) > 0.1f) ||
        (newPlayerInCombat != previousData.playerInCombat) ||
        levelIncreased;

    if (dataChanged) {
        outData.currentLevel = newLevel;
        outData.currentExp = newExp;
        outData.nextLevelExp = newNextLevelExp;
        outData.playerInCombat = newPlayerInCombat;
        outData.levelIncreased = levelIncreased;

        // Обновляем предыдущие данные
        previousData = outData;
        firstCall = false;

        return true;
    }

    return false;
}

bool ExperienceManager::GetUpdatedExperienceScript(std::string& outScript) {
    ExperienceData newData;

    if (GetUpdatedExperienceData(newData)) {
        // Конвертируем float в int для отправки
        int currentExpInt = static_cast<int>(newData.currentExp);
        int nextLevelExpInt = static_cast<int>(newData.nextLevelExp);

        outScript = "updateCircularExperienceData(" +
            std::to_string(newData.currentLevel) + "," +
            std::to_string(currentExpInt) + "," +
            std::to_string(nextLevelExpInt) + "," +
            std::to_string(newData.playerInCombat) + "," +
            std::to_string(newData.levelIncreased) + ")";
        return true;
    }

    return false;
}