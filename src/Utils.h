#include "RE/Skyrim.h"
#include <string>
#include <sstream>
#include <iomanip>

namespace Utils {
    std::string format_float(double value, int precision);
    float get_total_av(RE::Actor* a, RE::ActorValue av);
}
