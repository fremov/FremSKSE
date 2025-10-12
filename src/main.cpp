#include "SKSE/API.h"
#include "pch.h"
#include <cmath>
#include <algorithm> // Добавляем для std::max

float timeUpdateSkills;
RE::ActorValue actorValue;
int skillsValueCurrentLvl[] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };
float skillsValueCurrentExp[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

// Функция для округления в меньшую сторону
int FloorToInt(float value) {
    return static_cast<int>(std::floor(value));
}

// Получение текущего опыта навыка
float GetCurrentSkillExperience(RE::PlayerCharacter* actor, RE::ActorValue skill) {
    if (!actor || !actor->GetPlayerRuntimeData().skills) {
        return 0.0f;
    }

    int skillIndex = static_cast<int>(skill) - 6;
    if (skillIndex >= 0 && skillIndex < 18) {
        return actor->GetPlayerRuntimeData().skills->data->skills[skillIndex].xp;
    }

    return 0.0f;
}

float GetExperienceForLevel(RE::ActorValueInfo* info, int level) {
    double fSkillUseCurve = 0.0;
    const auto settings = RE::GameSettingCollection::GetSingleton();
    if (!settings)
        return 0.0;

    RE::Setting* skillUseCurve = settings->GetSetting("fSkillUseCurve");
    if (skillUseCurve)
        fSkillUseCurve = skillUseCurve->GetFloat();

    return info->skill->improveMult * (float)(pow(level, fSkillUseCurve)) + info->skill->improveOffset;
}

void ProcessSkillsUpdate(RE::PlayerCharacter* actor)
{
    if (!actor) return;

    RE::ActorValue skillCount[] = {
        RE::ActorValue::kArchery,
        RE::ActorValue::kOneHanded,
        RE::ActorValue::kTwoHanded,
        RE::ActorValue::kHeavyArmor,
        RE::ActorValue::kLightArmor,
        RE::ActorValue::kBlock,
        RE::ActorValue::kAlteration,
        RE::ActorValue::kIllusion,
        RE::ActorValue::kRestoration,
        RE::ActorValue::kDestruction,
        RE::ActorValue::kConjuration,
        RE::ActorValue::kSneak,
        RE::ActorValue::kPickpocket,
        RE::ActorValue::kAlchemy,
        RE::ActorValue::kLockpicking,
        RE::ActorValue::kEnchanting,
        RE::ActorValue::kSpeech,
        RE::ActorValue::kSmithing
    };

    const char* skillNames[] = {
        "Archery", "OneHanded", "TwoHanded", "HeavyArmor", "LightArmor",
        "Block", "Alteration", "Illusion", "Restoration", "Destruction",
        "Conjuration", "Sneak", "Pickpocket", "Alchemy", "Lockpicking",
        "Enchanting", "Speech", "Smithing"
    };

    for (int i = 0; i < 18; i++) {
        auto skill = skillCount[i];

        // Получаем текущий уровень и опыт
        float currentLevelFloat = actor->AsActorValueOwner()->GetActorValue(skill);
        int currentLevel = static_cast<int>(currentLevelFloat);
        float currentExperience = GetCurrentSkillExperience(actor, skill);

        // Общий накопленный опыт (для отслеживания прироста)
        float totalExperience = currentExperience;

        // Опыт для повышения навыка
        auto skillInfo = RE::ActorValueList::GetSingleton()->GetActorValue(skill);
        float currentActorValue = actor->AsActorValueOwner()->GetActorValue(skill);
        float currentExpForLevel = GetExperienceForLevel(skillInfo, static_cast<int>(currentActorValue + 1));

        // Проверяем изменения
        bool levelChanged = (currentLevel > skillsValueCurrentLvl[i]);
        bool expChanged = (totalExperience > skillsValueCurrentExp[i]);

        if (levelChanged || expChanged) {
            // Полученный опыт (округляем в меньшую сторону)
            int gainedExp = FloorToInt(totalExperience - skillsValueCurrentExp[i]);

            // Текущий опыт (округляем в меньшую сторону)
            int currentExpInt = FloorToInt(currentExperience);

            if (levelChanged) {
                int a = skillsValueCurrentLvl[i];
                gainedExp = 0;
                while(a < currentLevel) {
                    
                    a += 1;
                    gainedExp += GetExperienceForLevel(skillInfo, static_cast<int>(a));
                    logger::info("{} gainedExp: {} lvl {}", skillNames[i], gainedExp, a); // a - уровень навыка
                }
                gainedExp -= skillsValueCurrentExp[i];
                gainedExp += totalExperience;

                logger::info("SKILL LEVEL UP: {} {} -> {}",
                    skillNames[i], skillsValueCurrentLvl[i], currentLevel);
            }

            // Вывод в требуемом формате
            logger::info("{}: current exp {}, gained {}, level {} currentExpForLevel {}",
                skillNames[i],
                currentExpInt,     // текущий опыт в текущем уровне
                gainedExp,         // получено опыта с последней проверки
                currentLevel,      // текущий уровень навыка
                FloorToInt(currentExpForLevel) // опыт для лвл апа навыка
            );

            // Обновляем сохраненные значения
            skillsValueCurrentLvl[i] = currentLevel;
            skillsValueCurrentExp[i] = totalExperience;
        }
    }
}
// Остальной код без изменений
class PlayerUpdate
{
public:
    static void Hook()
    {
        _Update =
            REL::Relocation<uintptr_t>(RE::VTABLE_PlayerCharacter[0]).write_vfunc(0xad, Update);
    }

private:
    static void Update(RE::PlayerCharacter* player, float delta)
    {
        _Update(player, delta);
        if (timeUpdateSkills >= 3) {
            timeUpdateSkills = 0;
            ProcessSkillsUpdate(player);
        }
        else {
            timeUpdateSkills += delta;
        }
    }

    static inline REL::Relocation<decltype(Update)> _Update;
};

// Объявление внешних переменных
PRISMA_UI_API::IVPrismaUI1* PrismaUI = nullptr;
PrismaView view = 0;

static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message) {
    using namespace SKSE;
    using namespace RE;

    switch (message->type) {
    case MessagingInterface::kPostLoad:
        // Инициализируем PrismaUI API
        PrismaUI = static_cast<PRISMA_UI_API::IVPrismaUI1*>(
            PRISMA_UI_API::RequestPluginAPI(PRISMA_UI_API::InterfaceVersion::V1)
            );

        if (!PrismaUI) {
            logger::error("Failed to get PrismaUI API");
            return;
        }
        logger::info("PrismaUI API successfully initialized");
        break;

    case MessagingInterface::kDataLoaded:
        // Создаем PrismaUI view
        if (PrismaUI) {
            view = PrismaUI->CreateView("FremUI/index.html", [](PrismaView createdView) {
                logger::info("PrismaUI view created successfully");

                // Инициализируем все системы после создания view
                /*SkillWidget::Initialize();
                SkillWidget::Start();*/
                SKSE::GetTrampoline().create(228);
                PlayerUpdate::Hook();
                logger::info("PlayerUpdate successfully initialized");
                // Инициализируем систему лога урона
                //DamageLogManager::Initialize(PrismaUI, view);
                //WeaponHitHook::Install();
                //DamageLogManager::RegisterKeybinds();

                //// Регистрируем обработчики событий
                //Input::InputEventHandler::Register();
                //MenuHandler::register_();
                //SkillWidget::Initialize();

                // Запускаем поток обновления HUD
                HUDManager::g_running = true;
                std::thread(HUDManager::UpdateThread).detach();

                logger::info("All systems initialized successfully");
                });

            if (view == 0) {
                logger::error("Failed to create PrismaUI view");
            }
        }
        break;
    }
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse) {
    SKSE::Init(a_skse);

    auto messaging = SKSE::GetMessagingInterface();
    if (messaging) {
        messaging->RegisterListener("SKSE", SKSEMessageHandler);
    }
    else {
        logger::error("Failed to get messaging interface");
        return false;
    }

    return true;
}