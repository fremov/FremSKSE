#pragma once
#include "SKSE/API.h"
#include <atomic>
#include <thread>
#include <string>
#include <vector>

namespace SkillWidget
{
    struct SkillData
    {
        std::string name;
        int level;
        float currentExp;
        float maxExp;
        float maxExpSplit;
    };

    class SkillWidgetManager
    {
    public:
        static SkillWidgetManager& GetSingleton();

        void Initialize();
        void StartUpdateThread();
        void StopUpdateThread();

        bool IsVisible() const { return m_isVisible; }
        void SetVisible(bool visible) { m_isVisible = visible; }

        int GetScale() const { return m_scale; }
        void SetScale(int scale) { m_scale = scale; }

        float GetStep() const { return m_step; }
        void SetStep(float step) { m_step = step; }

        float GetTransparency() const { return m_transparency; }
        void SetTransparency(float alpha) { m_transparency = alpha; }

        void UpdateScale();
        void UpdateVisible();
        void UpdateTransparency();

    private:
        SkillWidgetManager() = default;
        ~SkillWidgetManager() = default;
        SkillWidgetManager(const SkillWidgetManager&) = delete;
        SkillWidgetManager& operator=(const SkillWidgetManager&) = delete;

        void UpdateThread();
        void UpdateSkills();
        void GetSkillData(int skillIndex, SkillData& skillData);
        float CalculateGainedExperience(int skillIndex, int currentLevel, float currentXP);
        bool ShouldShowSkills();
        bool IsInCombat();
        bool CheckInventoryMenu();
        int GetSkillArrayIndex(RE::ActorValue actorValue);

        // Properties
        std::atomic<bool> m_running = true;
        std::atomic<bool> m_isVisible = true;
        std::atomic<int> m_scale = 100;
        std::atomic<float> m_step = 40.0f;
        std::atomic<float> m_transparency = 100.0f;

        // Skill tracking
        std::vector<int> m_skillLevelOld;
        std::vector<float> m_previousSkillXP;

        // Skill names
        std::vector<std::string> m_skillNames = {
            "Marksman", "OneHanded", "TwoHanded", "HeavyArmor", "LightArmor",
            "Block", "Alteration", "Illusion", "Restoration", "Destruction",
            "Conjuration", "Sneak", "Pickpocket", "Alchemy", "Lockpicking",
            "Enchanting", "Speechcraft", "Smithing" };
    };

    void Initialize();
    void Start();
    void Stop();
}