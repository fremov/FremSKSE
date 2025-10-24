#pragma once
#include <RE/T/TESGlobal.h>
#include <RE/G/GameSettingCollection.h>
#include <RE/P/PlayerCharacter.h>
#include <string>

class ExperienceManager {
public:
    static ExperienceManager& GetInstance() {
        static ExperienceManager instance;
        return instance;
    }

    struct ExperienceData {
        int currentLevel;
        float currentExp;
        float nextLevelExp;
        bool playerInCombat;
        bool levelIncreased;
    };

    bool GetUpdatedExperienceData(ExperienceData& outData);
    bool GetUpdatedExperienceScript(std::string& outScript);

private:
    ExperienceManager() = default;

    float GetExperienceForLevel(int level);
    bool IsPlayerInCombat();

    ExperienceData previousData{ 0, 0.0f, 0.0f, false, false };
    bool firstCall = true;
};