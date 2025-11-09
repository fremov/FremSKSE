#pragma once

#include <string>
#include <unordered_set>
#include <queue>
#include <filesystem>
#include <fstream>
#include "json.hpp"
#include "include/PrismaUI_API.h"


using json = nlohmann::json;

class SimpleHintReader {
private:
    json data;

public:
    struct HintData {
        std::string name;
        std::string description;
    };

    bool load(const std::string& filename);
    HintData getHintData(const std::string& hintId);
};

class HintManager {
private:
    static inline std::unordered_set<std::string> shownHints;
    static inline bool isHintActive = false;
    static inline std::queue<std::string> pendingHints;
    SimpleHintReader reader;

    // Приватный конструктор для синглтона
    HintManager();
    std::string escapeForJavaScript(const std::string& input);

public:
    // Статический метод для получения экземпляра
    static HintManager& GetSingleton();

    void showHintOnce(const std::string& hintId);
    void onHintClosed();
};