#include "SKSE/API.h"
#include "pch.h"
#include <cmath>
#include <algorithm> // Добавляем для std::max
#include <MenuHandler.h>
#include "STB_Widgets_API.h"
#include "ExperienceWidget.h"

float timeUpdateSkills;
RE::ActorValue actorValue;
std::vector<int> skillsValueCurrentLvl = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
std::vector <float> skillsValueCurrentExp = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
std::vector<int> skillsValuePreviousLvl = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

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

    std::vector<std::string> skillsToSend;

    for (int i = 0; i < 18; i++) {
        auto skill = skillCount[i];

        // Получаем текущий уровень и опыт
        float currentLevelFloat = actor->AsActorValueOwner()->GetActorValue(skill);
        int currentLevel = static_cast<int>(currentLevelFloat);
        float currentExperience = GetCurrentSkillExperience(actor, skill);
        float totalExperience = currentExperience;

        // Опыт для повышения навыка
        auto skillInfo = RE::ActorValueList::GetSingleton()->GetActorValue(skill);
        float currentExpForLevel = GetExperienceForLevel(skillInfo, static_cast<int>(currentLevelFloat + 1));

        // Проверяем изменения
        bool levelChanged = (currentLevel > skillsValueCurrentLvl[i]);
        bool expChanged = (totalExperience > skillsValueCurrentExp[i]);

        // Отправляем только если есть изменения
        if (levelChanged || expChanged) {
            // Полученный опыт
            int gainedExp = FloorToInt(totalExperience - skillsValueCurrentExp[i]);
            int currentExpInt = FloorToInt(currentExperience);

            if (levelChanged) {
                int a = skillsValueCurrentLvl[i];
                gainedExp = 0;
                while (a < currentLevel) {
                    a += 1;
                    gainedExp += GetExperienceForLevel(skillInfo, static_cast<int>(a));
                }
                gainedExp -= skillsValueCurrentExp[i];
                gainedExp += totalExperience;
            }

            // Расчет прогресса
            float progress = 0.0f;
            if (currentExpForLevel > 0) {
                progress = (currentExperience / currentExpForLevel) * 100.0f;
                progress = min(progress, 100.0f);
            }

            // Формируем JSON для отправки в React
            std::string skillJson = "{";
            skillJson += "\"name\":\"" + std::string(skillNames[i]) + "\",";
            skillJson += "\"currentExp\":" + std::to_string(currentExpInt) + ",";
            skillJson += "\"gainedExp\":" + std::to_string(gainedExp) + ",";
            skillJson += "\"level\":" + std::to_string(currentLevel) + ",";
            skillJson += "\"expForNextLevel\":" + std::to_string(FloorToInt(currentExpForLevel)) + ",";
            skillJson += "\"progress\":" + std::to_string(progress);

            // Добавляем предыдущий уровень для анимации
            if (levelChanged) {
                skillJson += ",\"previousLevel\":" + std::to_string(skillsValueCurrentLvl[i]);
            }

            skillJson += "}";

            skillsToSend.push_back(skillJson);

            // Обновляем сохраненные значения
            skillsValueCurrentLvl[i] = currentLevel;
            skillsValueCurrentExp[i] = totalExperience;

            /*logger::info("{}: +{} опыта, уровень {} ({}%)",
                skillNames[i], gainedExp, currentLevel, progress);*/
        }
    }

    // Отправляем данные в React компонент только если есть изменения
    if (!skillsToSend.empty()) {
        std::string script = "fremUpdateSkills([";
        for (size_t i = 0; i < skillsToSend.size(); i++) {
            script += skillsToSend[i];
            if (i < skillsToSend.size() - 1) {
                script += ",";
            }
        }
        script += "])";

        if (PrismaUI && PrismaUI->IsValid(view)) {
            PrismaUI->Invoke(view, script.c_str());
            //logger::info("Отправлено {} измененных навыков", skillsToSend.size());
        }
    }
}

void ExperienceWidget() {
    std::string script;

    // Отправляем только если данные изменились
    if (ExperienceManager::GetInstance().GetUpdatedExperienceScript(script)) {
        if (PrismaUI && PrismaUI->IsValid(view)) {
            PrismaUI->Invoke(view, script.c_str());
            logger::info("ExperienceWidget {}", script);
        }
    }
}

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
        if (timeUpdateSkills >= 1) {
            ExperienceWidget(); // Теперь отправляет только при изменениях
        }
        if (timeUpdateSkills >= 3 && !(player->IsInCombat())) {
            timeUpdateSkills = 0;
            ProcessSkillsUpdate(player);
        }
        else {
            timeUpdateSkills += delta;
        }
    }

    static inline REL::Relocation<decltype(Update)> _Update;
};
class SaveMessage
	{
	public:
		static void Hook(SKSE::Trampoline& trampoline)
		{
			_ShowHUDMessage =
				trampoline.write_call<5>(REL::ID(50718).address() + 0x13ca,
					ShowHUDMessage);
			_ShowHUDMessage02 =
				trampoline.write_call<5>(REL::ID(39366).address() + 0x8f7,
					ShowHUDMessage02);
			_ShowHUDMessage03 =
				trampoline.write_call<5>(REL::ID(50737).address() + 0xaf,
					ShowHUDMessage03);
			_ShowHUDMessage04 =
				trampoline.write_call<5>(REL::ID(51842).address() + 0x4d,
					ShowHUDMessage04);
			_ShowHUDMessage05 =
				trampoline.write_call<5>(REL::ID(34862).address() + 0x3bb,
					ShowHUDMessage05);
		}

	private:
		static void ShowSaveWidget()
		{
			PrismaUI->Invoke(view, "data_from_skse_for_save_widget()");
			/*PrismaUI->Invoke(view, "set_save_widget_position_x(222)");
			PrismaUI->Invoke(view, "set_save_widget_position_y(600)");
			PrismaUI->Invoke(view, "set_save_widget_enabled(true)");
			PrismaUI->Invoke(view, "set_save_widget_always_visible(false)");*/
		}
		static void ShowHUDMessage(char* text, char* sound, char no_repeat)
		{
			ShowSaveWidget();
			return;
		}
		static inline REL::Relocation<decltype(ShowHUDMessage)> _ShowHUDMessage;
		static void ShowHUDMessage02(char* text, char* sound, char no_repeat)
		{
			ShowSaveWidget();
			return;
		}
		static inline REL::Relocation<decltype(ShowHUDMessage02)> _ShowHUDMessage02;
		static void ShowHUDMessage03(char* text, char* sound, char no_repeat)
		{
			ShowSaveWidget();
			return;
		}
		static inline REL::Relocation<decltype(ShowHUDMessage03)> _ShowHUDMessage03;
		static void ShowHUDMessage04(char* text, char* sound, char no_repeat)
		{
			ShowSaveWidget();
			return;
		}
		static inline REL::Relocation<decltype(ShowHUDMessage04)> _ShowHUDMessage04;
		static void ShowHUDMessage05(char* text, char* sound, char no_repeat)
		{
			ShowSaveWidget();
			return;
		}
		static inline REL::Relocation<decltype(ShowHUDMessage05)> _ShowHUDMessage05;
	};
// Объявление внешних переменных
PRISMA_UI_API::IVPrismaUI1* PrismaUI = nullptr;
PrismaView view = 0;
STB_UI_API::IVPrismaUI1* STBUI;

static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message) {
    using namespace SKSE;
    using namespace RE;

    switch (message->type) {
    case MessagingInterface::kPostLoad:
        // Инициализируем PrismaUI API
        if (!PrismaUI) {
            logger::error("Failed to get PrismaUI API");
            return;
        }
        logger::info("PrismaUI API successfully initialized");
        break;

    case MessagingInterface::kDataLoaded:
        // Создаем PrismaUI view
        PrismaUI = static_cast<PRISMA_UI_API::IVPrismaUI1*>(
            PRISMA_UI_API::RequestPluginAPI(PRISMA_UI_API::InterfaceVersion::V1)
            );
        STBUI = static_cast<STB_UI_API::IVPrismaUI1*>(
            STB_UI_API::RequestPluginAPI(STB_UI_API::InterfaceVersion::V1)
            );
        if (PrismaUI) {
            view = PrismaUI->CreateView("FremUI/index.html", [](PrismaView createdView) {
                logger::info("PrismaUI view created successfully");
                
                SKSE::GetTrampoline().create(228);

				PlayerUpdate::Hook(); // полоски навыков
                logger::info("PlayerUpdate successfully initialized");
				SaveMessage::Hook(SKSE::GetTrampoline()); // виджет сохранения
				logger::info("SaveMessage successfully initialized");
				MenuHandler::register_(); // резисты, лвл, интоксикация
				logger::info("MenuHandler successfully initialized");
                /*HUDManager::g_running = true;
                std::thread(HUDManager::UpdateThread).detach();
				logger::info("HUDManager update thread started");*/


                logger::info("All systems initialized successfully");
                });
			STBUI->GetView(view);

            if (view == 0) {
                logger::error("Failed to create PrismaUI view");
            }
        }
        break;
    }
}

void SaveCallback(SKSE::SerializationInterface* a_intfc)
{
    bool ok;
    ok = a_intfc->WriteRecord('DAT1', 315, (uint32_t)skillsValueCurrentExp.size());
    assert(ok);
    bool ok2;
    ok2 = a_intfc->WriteRecord('DAT2', 315, skillsValueCurrentExp.data(), (uint32_t)skillsValueCurrentExp.size() * sizeof(RE::FormID));
    assert(ok2);
    bool ok3;
    ok3 = a_intfc->WriteRecord('DAT3', 315, (uint32_t)skillsValueCurrentLvl.size());
    assert(ok3);
    bool ok4;
    ok4 = a_intfc->WriteRecord('DAT4', 315, skillsValueCurrentLvl.data(), (uint32_t)skillsValueCurrentLvl.size() * sizeof(RE::FormID));
    assert(ok4);
}

void LoadCallback(SKSE::SerializationInterface* a_intfc)
{
    bool ok;
    uint32_t type;
    uint32_t version;
    uint32_t length;
    uint32_t num = 0;
    while (a_intfc->GetNextRecordInfo(type, version, length)) {
        assert(version == 315);
        switch (type) {
        case 'DAT1':
            ok = a_intfc->ReadRecordData(num);
            assert(ok);
            skillsValueCurrentExp.resize(num);
            break;
        case 'DAT2':
            ok = a_intfc->ReadRecordData(skillsValueCurrentExp.data(), (uint32_t)skillsValueCurrentExp.size() * sizeof(RE::FormID));
            assert(ok);
            break;
        case 'DAT3':
            ok = a_intfc->ReadRecordData(num);
            assert(ok);
            skillsValueCurrentLvl.resize(num);
            break;
        case 'DAT4':
            ok = a_intfc->ReadRecordData(skillsValueCurrentLvl.data(), (uint32_t)skillsValueCurrentLvl.size() * sizeof(RE::FormID));
            assert(ok);
            break;
        default:
            assert(false);
            break;
        }
    }
}

void RevertCallback(SKSE::SerializationInterface*)
{
    skillsValueCurrentExp.clear();
    skillsValueCurrentLvl.clear();
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse) {
    SKSE::Init(a_skse);

    auto serialization = SKSE::GetSerializationInterface();
    serialization->SetUniqueID(0xfa13724);
    serialization->SetSaveCallback(SaveCallback);
    serialization->SetLoadCallback(LoadCallback);
    serialization->SetRevertCallback(RevertCallback);

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
