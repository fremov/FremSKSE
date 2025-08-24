#include "Utils.h"

namespace Utils {
    std::string format_float(double value, int precision) {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << value;
        return oss.str();
    }

    float Actor__GetActorValueModifier(RE::Actor* a, RE::ACTOR_VALUE_MODIFIER mod, RE::ActorValue av) {
        using func_t = decltype(Actor__GetActorValueModifier);
        REL::Relocation<func_t> func{ REL::ID(37524) };
        return func(a, mod, av);
    }

    float get_total_av(RE::Actor* a, RE::ActorValue av) {
        float permanent = a->AsActorValueOwner()->GetPermanentActorValue(av);
        float temporary = Actor__GetActorValueModifier(a, RE::ACTOR_VALUE_MODIFIER::kTemporary, av);
        return permanent + temporary;
    }
}
