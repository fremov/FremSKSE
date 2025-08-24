
//#include "Utils.h"
#include "SKSE/API.h"
#include <cmath>
#include <chrono>
#include "SkillWidget.h"
#include "PrismaUI_API.h"
#include "Utils.h"

extern PRISMA_UI_API::IVPrismaUI1 * PrismaUI;
extern PrismaView view;

namespace SkillWidget
{
    SkillWidgetManager& SkillWidgetManager::GetSingleton()
    {
        static SkillWidgetManager instance;
        return instance;
    }

    void SkillWidgetManager::Initialize()
    {
        m_skillLevelOld.resize(18, 1);
        m_previousSkillXP.resize(18, 0.0f);
        logger::info("SkillWidgetManager initialized");
    }

    void SkillWidgetManager::StartUpdateThread()
    {
        std::thread([this]()
            { UpdateThread(); })
            .detach();
    }

    void SkillWidgetManager::StopUpdateThread()
    {
        m_running = false;
    }

    void SkillWidgetManager::UpdateThread()
    {
        while (m_running)
        {
            UpdateSkills();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    void SkillWidgetManager::UpdateSkills()
    {
        if (!PrismaUI || view == 0) return;

        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player)
        {
            return;
        }

        std::string skillText;
        bool hasChanges = false;

        for (int skillIndex = 0; skillIndex < 18; ++skillIndex)
        {
            SkillData skillData;
            GetSkillData(skillIndex, skillData);

            // Проверяем изменения и отправляем данные только если есть изменения
            if (skillData.level != m_skillLevelOld[skillIndex] ||
                std::abs(skillData.currentExp - m_previousSkillXP[skillIndex]) > 0.1f)
            {
                hasChanges = true;

                // Отправляем данные только для изменившихся навыков
                std::string skillAll = std::to_string(skillIndex) + "," +
                    std::to_string(skillData.level) + "," +
                    std::to_string(static_cast<int>(std::round(skillData.currentExp))) + "," +
                    std::to_string(static_cast<int>(std::round(skillData.maxExp))) + "," +
                    std::to_string(static_cast<int>(std::round(skillData.maxExpSplit)));

                if (skillText.empty())
                {
                    skillText = skillAll;
                }
                else
                {
                    skillText += "-" + skillAll;
                }

                // Обновляем сохраненные значения
                m_skillLevelOld[skillIndex] = skillData.level;
                m_previousSkillXP[skillIndex] = skillData.currentExp;
            }
        }

        // Отправляем данные только если есть изменения
        if (hasChanges)
        {
            std::string script = "updateSkills('" + skillText + "')";
            PrismaUI->Invoke(view, script.c_str());

            // Логируем отправленные данные для отладки
            logger::info("Sent skill data: {}", script);
        }
    }

    void SkillWidgetManager::GetSkillData(int skillIndex, SkillData& skillData)
    {
        skillData.name = m_skillNames[skillIndex];

        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player || !player->GetPlayerRuntimeData().skills || !player->GetPlayerRuntimeData().skills->data)
        {
            skillData.level = 0;
            skillData.currentExp = 0.0f;
            skillData.maxExp = 0.0f;
            skillData.maxExpSplit = 0.0f;
            return;
        }

        // Regular Skyrim skills - map to correct ActorValue
        RE::ActorValue actorValue;
        switch (skillIndex)
        {
        case 0: actorValue = RE::ActorValue::kArchery; break;
        case 1: actorValue = RE::ActorValue::kOneHanded; break;
        case 2: actorValue = RE::ActorValue::kTwoHanded; break;
        case 3: actorValue = RE::ActorValue::kHeavyArmor; break;
        case 4: actorValue = RE::ActorValue::kLightArmor; break;
        case 5: actorValue = RE::ActorValue::kBlock; break;
        case 6: actorValue = RE::ActorValue::kAlteration; break;
        case 7: actorValue = RE::ActorValue::kIllusion; break;
        case 8: actorValue = RE::ActorValue::kRestoration; break;
        case 9: actorValue = RE::ActorValue::kDestruction; break;
        case 10: actorValue = RE::ActorValue::kConjuration; break;
        case 11: actorValue = RE::ActorValue::kSneak; break;
        case 12: actorValue = RE::ActorValue::kPickpocket; break;
        case 13: actorValue = RE::ActorValue::kAlchemy; break;
        case 14: actorValue = RE::ActorValue::kLockpicking; break;
        case 15: actorValue = RE::ActorValue::kEnchanting; break;
        case 16: actorValue = RE::ActorValue::kSpeech; break;
        case 17: actorValue = RE::ActorValue::kSmithing; break;
        default: actorValue = RE::ActorValue::kArchery; break;
        }

        int skillArrayIndex = GetSkillArrayIndex(actorValue);
        auto skillsData = player->GetPlayerRuntimeData().skills->data;

        if (skillArrayIndex >= 0 && skillArrayIndex < 18)
        {
            auto& skill = skillsData->skills[skillArrayIndex];

            skillData.level = skill.level;
            skillData.currentExp = skill.xp;
            skillData.maxExp = skill.levelThreshold;
            skillData.maxExpSplit = CalculateGainedExperience(skillIndex, skill.level, skill.xp);
        }
        else
        {
            skillData.level = 0;
            skillData.currentExp = 0.0f;
            skillData.maxExp = 0.0f;
            skillData.maxExpSplit = 0.0f;
        }
    }

    float SkillWidgetManager::CalculateGainedExperience(int skillIndex, int currentLevel, float currentXP)
    {
        int oldLevel = m_skillLevelOld[skillIndex];
        float oldXP = m_previousSkillXP[skillIndex];

        // Если нет изменений, возвращаем 0
        if (currentLevel == oldLevel && std::abs(currentXP - oldXP) < 0.1f)
        {
            return 0.0f;
        }

        float gainedXP = 0.0f;

        // Если уровень не изменился, просто вычисляем разницу в опыте
        if (currentLevel == oldLevel)
        {
            gainedXP = currentXP - oldXP;
        }
        else
        {
            // Вычисляем опыт, полученный на старом уровне
            float maxXPForOldLevel = 1.54f * std::pow(static_cast<float>(oldLevel), 2.15f) + 154.0f;
            gainedXP = maxXPForOldLevel - oldXP;

            // Добавляем опыт за промежуточные уровни
            for (int level = oldLevel + 1; level < currentLevel; ++level)
            {
                float maxXPForLevel = 1.54f * std::pow(static_cast<float>(level), 2.15f) + 154.0f;
                gainedXP += maxXPForLevel;
            }

            // Добавляем опыт на текущем уровне
            gainedXP += currentXP;
        }

        return gainedXP;
    }

    int SkillWidgetManager::GetSkillArrayIndex(RE::ActorValue actorValue)
    {
        return static_cast<int>(actorValue) - 6;
    }

    bool SkillWidgetManager::ShouldShowSkills()
    {
        if (!m_isVisible)
        {
            return false;
        }

        // Проверяем изменения навыков
        auto player = RE::PlayerCharacter::GetSingleton();
        if (!player || !player->GetPlayerRuntimeData().skills || !player->GetPlayerRuntimeData().skills->data)
        {
            return false;
        }

        auto skillsData = player->GetPlayerRuntimeData().skills->data;
        bool hasChanges = false;

        for (int i = 0; i < 18; ++i)
        {
            RE::ActorValue actorValue;
            switch (i)
            {
            case 0: actorValue = RE::ActorValue::kArchery; break;
            case 1: actorValue = RE::ActorValue::kOneHanded; break;
            case 2: actorValue = RE::ActorValue::kTwoHanded; break;
            case 3: actorValue = RE::ActorValue::kHeavyArmor; break;
            case 4: actorValue = RE::ActorValue::kLightArmor; break;
            case 5: actorValue = RE::ActorValue::kBlock; break;
            case 6: actorValue = RE::ActorValue::kAlteration; break;
            case 7: actorValue = RE::ActorValue::kIllusion; break;
            case 8: actorValue = RE::ActorValue::kRestoration; break;
            case 9: actorValue = RE::ActorValue::kDestruction; break;
            case 10: actorValue = RE::ActorValue::kConjuration; break;
            case 11: actorValue = RE::ActorValue::kSneak; break;
            case 12: actorValue = RE::ActorValue::kPickpocket; break;
            case 13: actorValue = RE::ActorValue::kAlchemy; break;
            case 14: actorValue = RE::ActorValue::kLockpicking; break;
            case 15: actorValue = RE::ActorValue::kEnchanting; break;
            case 16: actorValue = RE::ActorValue::kSpeech; break;
            case 17: actorValue = RE::ActorValue::kSmithing; break;
            default: actorValue = RE::ActorValue::kArchery; break;
            }

            int skillArrayIndex = GetSkillArrayIndex(actorValue);
            if (skillArrayIndex >= 0 && skillArrayIndex < 18)
            {
                auto& skill = skillsData->skills[skillArrayIndex];
                if (skill.level != m_skillLevelOld[i] || std::abs(skill.xp - m_previousSkillXP[i]) > 0.1f)
                {
                    hasChanges = true;
                    break;
                }
            }
        }

        if (!hasChanges)
        {
            return false;
        }

        // Не показываем во время боя
        if (IsInCombat())
        {
            return false;
        }

        return true;
    }

    bool SkillWidgetManager::IsInCombat()
    {
        auto player = RE::PlayerCharacter::GetSingleton();
        return player && player->IsInCombat();
    }

    bool SkillWidgetManager::CheckInventoryMenu()
    {
        auto ui = RE::UI::GetSingleton();
        if (!ui)
        {
            return true;
        }

        // Только критические меню, которые должны скрывать навыки
        static const std::vector<std::string> menuNames = {
            "StatsMenu",
            "Console",
            "Main Menu" };

        for (const auto& menuName : menuNames)
        {
            if (ui->IsMenuOpen(menuName))
            {
                return false;
            }
        }

        return true;
    }

    void SkillWidgetManager::UpdateScale()
    {
        if (PrismaUI->IsValid(view))
        {
            std::string script = "setSkillScale(" + std::to_string(m_scale) + ")";
            PrismaUI->Invoke(view, script.c_str());
        }
    }

    void SkillWidgetManager::UpdateVisible()
    {
        if (!PrismaUI || view == 0)
        {
            std::string visibility = m_isVisible ? "true" : "false";
            std::string script = "setSkillVisible(" + visibility + ")";
            PrismaUI->Invoke(view, script.c_str());
        }
    }

    void SkillWidgetManager::UpdateTransparency()
    {
        if (!PrismaUI || view == 0) return;
        {
            std::string script = "setSkillTransparency(" + std::to_string(m_transparency) + ")";
            PrismaUI->Invoke(view, script.c_str());
        }
    }

    void Initialize()
    {
        SkillWidgetManager::GetSingleton().Initialize();
    }

    void Start()
    {
        SkillWidgetManager::GetSingleton().StartUpdateThread();
    }

    void Stop()
    {
        SkillWidgetManager::GetSingleton().StopUpdateThread();
    }
}