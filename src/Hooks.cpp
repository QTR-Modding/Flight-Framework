#include "Hooks.h"

#include "FlightManager.h"
#include "Menu.h"
#include "PlayerBody.h"
#include "Utils.h"
#include "SkyPromptClient.h"
#include "PromptManager.h"
#include "FlightConditionsManager.h"
#include "stl.h"



struct ModActorValueHook {
    static bool thunk(RE::Actor* a_target, int a_2, RE::ActorValue a_av, float a_mod, RE::Actor* a_source) {
        if (
            a_mod < 0 &&
            a_target && 
            a_target->IsPlayerRef() && 
            a_av == RE::ActorValue::kStamina && 
            !RE::PlayerCharacter::GetSingleton()->IsPowerAttacking() &&
            FlightManager::IsFlying()
        ) {
            a_mod = 0;
        }
        return originalFunction(a_target, a_2, a_av, a_mod, a_source);
    }
    static inline REL::Relocation<decltype(thunk)> originalFunction;
    static void Install() { 
        originalFunction = stl::write_prologue_hook(REL::RelocationID(37523, 38468).address(), thunk);
    }
};

struct Update {
    static inline RE::TESObjectWEAP* thunk(RE::Actor* actor, char hand) {
        auto result = originalFunction(actor, hand);
        if (actor && actor->IsPlayerRef() && !Menu::IsOpen()) {
            bool shouldShow = false;
            for (auto perk : FlightConditionsManager::GetPerks()) {
                if (perk->perkConditions.IsTrue(actor, actor)) { 
                    shouldShow = true;
                } 
            }

            if (!FlightManager::IsFlying()) {
                Utils::SetPlayerFlying(false);
            }

            if (shouldShow) {
                PromptManager::EnableFligt();
            } else {
                if (FlightManager::IsFlying()) {
                    FlightManager::ExitFlight();
                }
                PromptManager::DisableFlight();
            }

            // https://github.com/fenix31415/UselessFenixUtils/blob/77d03948b78ec721939e589283ef9f0c8711327d/src/DebugRenderUtils.cpp#L1373C1-L1374C53
            float* g_deltaTime = (float*)REL::RelocationID(523660, 410199).address();
            FlightManager::Tick(*g_deltaTime);
        }

        return result;
    }
    static inline REL::Relocation<decltype(thunk)> originalFunction;
    static inline void Install() {
        auto& trampoline = SKSE::GetTrampoline();
        const REL::Relocation<std::uintptr_t> function{REL::RelocationID(36357, 37348)};
        originalFunction = trampoline.write_call<5>(function.address() + REL::Relocate(0x98, 0x91), thunk);
    }
};


struct FallHook {
    static void thunk(RE::PlayerControls* a1, int32_t a2, int32_t a3) {
        if (FlightManager::GetCurrentState() != nullptr) {
            return;
        }

        originalFunction(a1, a2, a3);
    }
    static inline REL::Relocation<decltype(thunk)> originalFunction;
    static inline void Install() {
        auto& trampoline = SKSE::GetTrampoline();
        originalFunction = trampoline.write_call<5>(REL::RelocationID(36507, 37507).address() + REL::Relocate(0x907, 0xa19), thunk);
    }
};


void Hooks::Install() {
    SKSE::AllocTrampoline(14 * 2);
    Update::Install();
    FallHook::Install();
    ModActorValueHook::Install();
}
