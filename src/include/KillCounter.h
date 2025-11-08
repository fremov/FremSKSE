class KillCounter {
public:
    static void Increment() {
        killCount++;
        UpdateUI();
        logger::info("count: {}", killCount);
    }

    static void Reset() {
        killCount = 0;
        UpdateUI();
        logger::info("Kill counter reset");
    }

    static int GetCount() {
        return killCount;
    }

private:
    static void UpdateUI() {
        if (PrismaUI && view) {
            std::string script = "updateKillCounter(" + std::to_string(killCount) + ")";
            PrismaUI->Invoke(view, script.c_str());
        }
    }

    static inline int killCount = 0;
};