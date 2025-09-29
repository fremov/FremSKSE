namespace Adresses {
    static inline REL::Relocation<uintptr_t> on_effect_added_npc{ RELOCATION_ID(261401, 0) };
    static inline REL::Relocation<uintptr_t> on_effect_added_pc{ RELOCATION_ID(261920, 0) };
}
namespace Offsets {
    static inline auto on_effect_added_npc = RELOCATION_OFFSET(0x08, 0);
    static inline auto on_effect_added_pc = RELOCATION_OFFSET(0x08, 0);
}

struct OnEffectAdded final
{
public:
    static auto install_hook() -> void
    {
        logger::info("start hook OnEffectAdded"sv);
        effect_added_npc_ = Adresses::on_effect_added_npc.write_vfunc(Offsets::on_effect_added_npc, effect_added_npc);
        effect_added_pc_ = Adresses::on_effect_added_pc.write_vfunc(Offsets::on_effect_added_pc, effect_added_pc);
        logger::info("finish hook OnEffectAdded"sv);
    }

private:
    static auto effect_added(RE::MagicTarget* this_, RE::ActiveEffect* active_effect) -> void;

    static auto effect_added_npc(RE::MagicTarget* this_, RE::ActiveEffect* active_effect) -> void;
    static auto effect_added_pc(RE::MagicTarget* this_, RE::ActiveEffect* active_effect) -> void;

    static inline REL::Relocation<decltype(effect_added_npc)> effect_added_npc_;
    static inline REL::Relocation<decltype(effect_added_pc)> effect_added_pc_;
};