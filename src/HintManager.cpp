#include "include/HintManager.h"
#include <include/MenuHandler.h>


bool SimpleHintReader::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        logger::info("Err open json");
        return false;
    }

    try {
        data = json::parse(file);
        logger::info("JSON loaded successfully: {} hints", data.size());
        return true;
    }
    catch (const std::exception& e) {
        logger::info("Err read json: {}", e.what());
        return false;
    }
}

SimpleHintReader::HintData SimpleHintReader::getHintData(const std::string& hintId) {
    for (const auto& item : data) {
        if (item.contains("id") && item["id"] == hintId) {
            HintData result;
            if (item.contains("name")) {
                result.name = item["name"];
            }
            if (item.contains("description")) {
                result.description = item["description"];
            }
            return result;
        }
    }
    return { "", "" }; 
}

HintManager::HintManager() {
    std::string currentPath = std::filesystem::current_path().string();
    if (!reader.load(currentPath + "/[STB] Mod Organizer/mods/STB Widgets/SKSE/Plugins/StorageUtilData/data_hints.json")) {
        logger::info("Error load json");
    }
}

HintManager& HintManager::GetSingleton() {
    static HintManager instance;
    return instance;
}

void HintManager::showHintOnce(const std::string& hintId) {
    if (shownHints.count(hintId) > 0) {
        return;
    }

    // Если уже есть активная подсказка, добавляем в очередь
    if (isHintActive) {
        pendingHints.push(hintId);
        logger::info("Hint queued: {}", hintId);
        return;
    }

    auto hintData = reader.getHintData(hintId);
    if (!hintData.name.empty() && !hintData.description.empty()) {
        // Устанавливаем флаг активности
        isHintActive = true;

        // Экранируем для JavaScript
        std::string escaped_name = escapeForJavaScript(hintData.name);
        std::string escaped_description = escapeForJavaScript(hintData.description);

        // Показываем новую подсказку
        std::string script = "window.receiveHintData('" +
            escaped_name + "', `" + escaped_description + "`);";

        PrismaUI->Invoke(view, script.c_str());
        logger::info("hint_script {}", script);
        shownHints.insert(hintId);

        logger::info("Showing hint: {} - {}", hintId, hintData.name);
    }
}

void HintManager::onHintClosed() {
    isHintActive = false;
    logger::info("Hint closed, active: false");

    // Если есть ожидающие подсказки, показываем следующую
    if (!pendingHints.empty()) {
        std::string nextHintId = pendingHints.front();
        pendingHints.pop();

        // Даем немного времени на закрытие текущей подсказки
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        showHintOnce(nextHintId);
    }
}

std::string HintManager::escapeForJavaScript(const std::string& input) {
    std::string result;
    for (char c : input) {
        switch (c) {
        case '\\': result += "\\\\"; break;
        case '\'': result += "\\'"; break;
        case '\"': result += "\\\""; break;
        case '`': result += "\\`"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default: result += c; break;
        }
    }
    return result;
}