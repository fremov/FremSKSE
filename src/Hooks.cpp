//#include "Hooks.h"
//
//auto FremHooks::OnEffectAdded::effect_added(RE::MagicTarget* this_, RE::ActiveEffect* active_effect) -> void
//{
//    if (!active_effect) {
//        return;
//    }
//
//    auto caster_handle = active_effect->caster;
//    auto target_handle = active_effect->target;
//
//    if (caster_handle && caster_handle.get() && caster_handle.get().get() && target_handle && active_effect->effect &&
//        active_effect->effect->baseEffect) {
//        const auto target = (RE::Actor*)((char*)this_ - 0x98);
//        if (caster_handle.get()->IsPlayerRef() || (target && target->IsPlayerRef())) {
//            FremHooks::console_log(
//                std::format("EFFECT_ADDED: Aggressor: {} Target: {} SpellName: {} SourceName: {} EffectName: {} "
//                    "EffectEditorId: {} Magnitude: {}: Duration: {}",
//                    caster_handle.get()->GetDisplayFullName(),
//                    target->GetDisplayFullName(),
//                    active_effect->spell ? active_effect->spell->GetFullName() : "None",
//                    active_effect->source ? active_effect->source->GetName() : "None",
//                    active_effect->effect->baseEffect->GetFullName(),
//                    active_effect->effect->baseEffect->GetFormEditorID(),
//                    active_effect->GetMagnitude(),
//                    active_effect->duration));
//        }
//    }
//}
//
//auto FremHooks::OnEffectAdded::effect_added_npc(RE::MagicTarget* this_, RE::ActiveEffect* active_effect) -> void
//{
//
//    //effect_added(this_, active_effect);
//
//    if (!this_ || !active_effect) {
//        return effect_added_npc_(this_, active_effect);
//    }
//
//     /*if (active_effect->effect && active_effect->effect->baseEffect && active_effect->magnitude > 0) {
//       logger::info("Effect: {} {} {}"sv,
//            active_effect->effect->baseEffect->GetName(),
//            active_effect->GetMagnitude(),
//            active_effect->duration);
//     }*/
//
//    return effect_added_npc_(this_, active_effect);
//}
//
//auto FremHooks::OnEffectAdded::effect_added_pc(RE::MagicTarget* this_, RE::ActiveEffect* active_effect) -> void
//{
//
//    effect_added(this_, active_effect);
//
//    if (!this_ || !active_effect) {
//        return effect_added_pc_(this_, active_effect);
//    }
//
//     if (active_effect->effect && active_effect->effect->baseEffect) {
//         logger::info("effect_added_pc {} {} {}"sv,
//            active_effect->effect->baseEffect->GetName(),
//            active_effect->GetMagnitude(),
//            active_effect->duration);
//     }
//
//    return effect_added_pc_(this_, active_effect);
//}