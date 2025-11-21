#include "SKSE/API.h"
#include "include/pch.h"


using json = nlohmann::json;

float timeUpdateSkills;
RE::ActorValue actorValue;
std::vector<int> skillsValueCurrentLvl = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
std::vector <float> skillsValueCurrentExp = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
std::vector<int> skillsValuePreviousLvl = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

RE::TESFaction* Falkreath;
RE::TESFaction* Pale;
RE::TESFaction* Winterhold;
RE::TESFaction* Haafingar;
RE::TESFaction* Whiterun;
RE::TESFaction* Reach;
RE::TESFaction* Eastmarch;
RE::TESFaction* Rift;
RE::TESFaction* Hjaalmarch;

using json = nlohmann::json;

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

std::string buildCrimeDataScript() {
    std::vector<std::string> crimeData;

    // Функция для добавления данных холда (всегда, даже если 0)
    auto addHoldData = [&](RE::TESFaction* faction, const std::string& holdKey) {
        if (faction) {
            int crimeGold = faction->GetCrimeGold();
            // Всегда добавляем данные, даже если штраф 0
            crimeData.push_back("\"" + holdKey + "\":" + std::to_string(crimeGold));
        }
        else {
            // Если фракция не найдена, отправляем 0
            crimeData.push_back("\"" + holdKey + "\":0");
        }
        };

    // Добавляем ВСЕ холды
    addHoldData(Falkreath, "falkreath");
    addHoldData(Pale, "pale");
    addHoldData(Winterhold, "winterhold");
    addHoldData(Haafingar, "haafingar");
    addHoldData(Whiterun, "whiterun");
    addHoldData(Reach, "reach");
    addHoldData(Eastmarch, "eastmarch");
    addHoldData(Rift, "rift");
    addHoldData(Hjaalmarch, "hjaalmarch");

    // Формируем JSON строку
    std::string jsonData = "{";
    for (size_t i = 0; i < crimeData.size(); ++i) {
        jsonData += crimeData[i];
        if (i < crimeData.size() - 1) {
            jsonData += ",";
        }
    }
    jsonData += "}";

    // Формируем скрипт с JSON строкой
    std::string script = "updateCrimeWidgets('" + jsonData + "')";
    return script;
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

        //static bool initialized = false;

        //// Инициализируем для подсказок 
        //if (!initialized) {
        //    initialized = true;
        //}
      
        // виджет штрафов
        if (timeUpdateSkills >= 3 && !(player->IsInCombat())) {
            PrismaUI->Invoke(view, buildCrimeDataScript().c_str());
        }
        
        // При весе 95% от макс
        /*float currentWeight = player->AsActorValueOwner()->GetActorValue(RE::ActorValue::kCarryWeight);
        float maxWeight = player->AsActorValueOwner()->GetPermanentActorValue(RE::ActorValue::kCarryWeight);
        if (currentWeight / maxWeight >= 0.95f) {
            HintManager::GetSingleton().showHintOnce("backpack");
        }*/

        // полоски навыков
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

PRISMA_UI_API::IVPrismaUI1* PrismaUI = nullptr;
PrismaView view = 0;
STB_UI_API::IVPrismaUI1* STBUI;

std::string GetString(const std::vector<RE::BSFixedString> a_text) {
    if (a_text.empty()) {
        return "";
    }

    std::string result = a_text[0].c_str(); 

    for (size_t i = 1; i < a_text.size(); ++i) {
        std::string text = a_text[i].c_str();
        result += "|" + text;
    }

    return result;
}

void FremShowChoiceMsg(RE::StaticFunctionTag*, std::vector<RE::BSFixedString> text)
{
    logger::info("FremShowChoiceMsg {}", GetString(text));
	PrismaUI->Invoke(view, ("updateItemsList('" + GetString(text) + "')").c_str());
	PrismaUI->Focus(view);
}

bool FremPapyrusFunctions(RE::BSScript::IVirtualMachine* vm)
{
    vm->RegisterFunction("FremShowChoiceMsg", "STB_Functions", FremShowChoiceMsg);
    return true;
}

static void SKSEMessageHandler(SKSE::MessagingInterface::Message* message) {
    using namespace SKSE;
    using namespace RE;

    switch (message->type) {
    case MessagingInterface::kPostLoad: 
        if (!PrismaUI) {
            logger::error("Failed to get PrismaUI API");
            return;
        }
        logger::info("PrismaUI API successfully initialized");
        break;

    case MessagingInterface::kDataLoaded:

        SKSE::GetPapyrusInterface()->Register(FremPapyrusFunctions);

	    Falkreath = TESForm::LookupByID<TESFaction>(0x28170);
        Pale = TESForm::LookupByID<TESFaction>(0x2816E);
	    Winterhold = TESForm::LookupByID<TESFaction>(0x2816F);
	    Haafingar = TESForm::LookupByID<TESFaction>(0x29DB0);
	    Whiterun = TESForm::LookupByID<TESFaction>(0x267EA);
	    Reach = TESForm::LookupByID<TESFaction>(0x2816C);
	    Eastmarch = TESForm::LookupByID<TESFaction>(0x267E3);
	    Rift = TESForm::LookupByID<TESFaction>(0x2816B);
	    Hjaalmarch = TESForm::LookupByID<TESFaction>(0x2816D);

        PrismaUI = static_cast<PRISMA_UI_API::IVPrismaUI1*>(PRISMA_UI_API::RequestPluginAPI(PRISMA_UI_API::InterfaceVersion::V1));
        STBUI = static_cast<STB_UI_API::IVPrismaUI1*>(STB_UI_API::RequestPluginAPI(STB_UI_API::InterfaceVersion::V1));

        if (PrismaUI) {
            view = PrismaUI->CreateView("FremUI/index.html", [](PrismaView createdView) {
                logger::info("PrismaUI view created successfully");
                
                SKSE::GetTrampoline().create(500);

				PlayerUpdate::Hook(); 
                logger::info("PlayerUpdate successfully initialized");

				SaveMessage::Hook(SKSE::GetTrampoline()); 
				logger::info("SaveMessage successfully initialized");

				MenuHandler::register_(); 
				logger::info("MenuHandler successfully initialized");

                Input::InputEventHandler::Register();
                logger::info("InputEventHandler successfully initialized");

                });
                
                PrismaUI->RegisterJSListener(view, "sendMenuDataSKSE", [](const char* data) -> void {
                    std::string str(data);
                    int comma = str.find(',');
                    int index = std::stoi(str.substr(0, comma));
                    int count = std::stoi(str.substr(comma + 1));
                    SKSE::ModCallbackEvent Event;
                    Event.eventName = "SpawnMenuClosed";
                    Event.strArg = data;
                    Event.numArg = 0;
                    Event.sender = nullptr;
                    auto modCallback = SKSE::GetModCallbackEventSource();
                    if (modCallback) {
                        modCallback->SendEvent(&Event); 
                    }
                    logger::info("Index: {}, Count: {}", index, count);
					PrismaUI->Unfocus(view);
                });

			    STBUI->GetView(view);
                logger::info("STB_API: {}", view);
                logger::info("All systems initialized successfully");
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
