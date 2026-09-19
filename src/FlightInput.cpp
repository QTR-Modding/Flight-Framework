#include "FlightInput.h"

#include <array>
#include <chrono>
#include <map>
#include <string>
#include <string_view>

#include "FlightManager.h"
#include "Configuration.h"
#include "InputConfig.h"
#include "Menu.h"
#include "PromptManager.h"
#include "SKSEMenuFramework.h"
#include "SkyPromptClient.h"

namespace {
    using Button = std::pair<RE::INPUT_DEVICE, uint32_t>;
    struct ActivePress {
        std::string action;
        bool modified;
        bool pendingHold = false;
    };

    using Clock = std::chrono::steady_clock;
    constexpr auto doublePressWindow = std::chrono::milliseconds(300);
    struct PendingTap {
        std::string action;
        std::string userEvent;
        Clock::time_point deadline;
        bool released = false;
        bool modified = false;
    };

    std::map<Button, PendingTap> pendingTaps;
    std::map<Button, ActivePress> consumedButtons;
    constexpr float hiddenPromptHoldSeconds = 0.5f;
    bool backHeld = false;
    bool backUsed = false;
    bool backArmed = false;
    constexpr auto gamepad = RE::INPUT_DEVICE::kGamepad;
    constexpr uint32_t backKey = RE::BSWin32GamepadDevice::Key::kBack;
    constexpr std::array actions = {
        "skyprompt.fly", "skyprompt.land", "skyprompt.fall", "ui.flyUp", "ui.flyDown"
    };

    bool GameplayAvailable()
    {
        return RE::PlayerCharacter::GetSingleton() && !Menu::IsOpen() &&
            !SKSEMenuFramework::IsAnyBlockingWindowOpened();
    }

    bool ActionAvailable(const std::string& action)
    {
        if (!PromptManager::canFly || !PromptManager::isVisible) {
            return false;
        }
        if (action == "skyprompt.fly") {
            return !PromptManager::isFlying;
        }
        return PromptManager::isFlying && FlightManager::IsFlying();
    }

    bool UsesBackModifier()
    {
        for (const auto action : actions) {
            for (const auto& binding : InputConfig::GetBindings(action)) {
                if (binding.device == gamepad && binding.modifier == backKey) {
                    return true;
                }
            }
        }
        return false;
    }

    void UpdateVerticalInput()
    {
        bool up = false;
        bool down = false;
        for (const auto& [button, press] : consumedButtons) {
            up |= press.action == "ui.flyUp";
            down |= press.action == "ui.flyDown";
        }
        if (up) {
            FlightManager::UpPressed();
        } else {
            FlightManager::UpReleased();
        }
        if (down) {
            FlightManager::DowmPressed();
        } else {
            FlightManager::DowmReleased();
        }
    }

    void Activate(const std::string& action)
    {
        if (action == "skyprompt.fly") {
            PromptManager::StartFlying();
        } else if (action == "skyprompt.land") {
            PromptManager::ExitFlight();
        } else if (action == "skyprompt.fall") {
            PromptManager::StopFlying();
        }
        UpdateVerticalInput();
    }

    bool ReplaySinglePress(const Button& key, const PendingTap& tap)
    {
        if (tap.userEvent.empty()) {
            return true; // No native action is mapped to this button.
        }
        auto playerControls = RE::PlayerControls::GetSingleton();
        auto menuControls = RE::MenuControls::GetSingleton();
        auto source = RE::BSInputDeviceManager::GetSingleton();
        if (!playerControls || !menuControls || !source) {
            logger::error("Cannot replay single press: native input controls unavailable.");
            return false;
        }
        auto button = RE::ButtonEvent::Create(key.first, RE::BSFixedString(tap.userEvent.c_str()),
            key.second, 1.0f, 0.0f);
        if (!button) {
            logger::error("Cannot allocate delayed single-press event.");
            return false;
        }
        // Dispatch directly to native consumers, bypassing our double-tap filter.
        // The game's own jump, interaction, and menu restrictions still apply.
        auto playerSink = static_cast<RE::BSTEventSink<RE::InputEvent*>*>(playerControls);
        RE::InputEvent* event = button;
        playerSink->ProcessEvent(&event, source);
        menuControls->ProcessEvent(&event, source);
        if (tap.released) {
            button->GetRuntimeData().value = 0.0f;
            button->GetRuntimeData().heldDownSecs = 0.01f;
            playerSink->ProcessEvent(&event, source);
            menuControls->ProcessEvent(&event, source);
        }
        button->~ButtonEvent();
        RE::free(button);
        return true;
    }

    void FlushExpiredTaps()
    {
        const auto now = Clock::now();
        // Remove each entry before native dispatch, which may open a menu and
        // synchronously cancel the remaining pending taps.
        while (true) {
            auto expired = pendingTaps.end();
            for (auto it = pendingTaps.begin(); it != pendingTaps.end(); ++it) {
                if (!ActionAvailable(it->second.action) || now >= it->second.deadline) {
                    expired = it;
                    break;
                }
            }
            if (expired == pendingTaps.end()) {
                break;
            }
            const auto key = expired->first;
            const auto tap = expired->second;
            pendingTaps.erase(expired);
            if (!GameplayAvailable() || !ActionAvailable(tap.action)) {
                continue;
            }
            // Modified gestures never leak their main button's native action.
            if (!tap.modified && ReplaySinglePress(key, tap)) {
                // If still held, allow subsequent held/up events through.
                consumedButtons.erase(key);
            }
        }
    }

    void ProcessDoublePress(const Button& key, const std::string& action,
        const ResolvedInputBinding& binding)
    {
        const auto previous = pendingTaps.find(key);
        // Update() expired deadlines before processing this input event. Use
        // that decision consistently, including presses right at the boundary.
        if (previous != pendingTaps.end() && previous->second.released &&
            previous->second.action == action && previous->second.modified == binding.modifier.has_value()) {
            pendingTaps.erase(previous);
            consumedButtons[key] = { action, binding.modifier.has_value() };
            Activate(action);
            return;
        }
        auto controlMap = RE::ControlMap::GetSingleton();
        if (!controlMap) {
            logger::error("Cannot defer single press: control map unavailable.");
            return;
        }
        pendingTaps[key] = {
            action, std::string(controlMap->GetUserEventName(key.second, key.first)),
            Clock::now() + doublePressWindow, false, binding.modifier.has_value()
        };
        consumedButtons[key] = { "", binding.modifier.has_value() };
    }

    void ReplayWaitTap(RE::ButtonEvent* button)
    {
        auto controls = RE::MenuControls::GetSingleton();
        auto source = RE::BSInputDeviceManager::GetSingleton();
        if (!controls || !source) {
            logger::error("Cannot replay Back/View tap: menu controls or input manager unavailable.");
            return;
        }

        // Reuse the live event only during synchronous dispatch. Sending it to
        // MenuControls preserves native Wait checks and avoids our input filter.
        const auto savedData = button->GetRuntimeData();
        const auto savedUserEvent = button->GetUserEvent();
        auto savedNext = button->next;
        button->next = nullptr;
        button->SetUserEvent(RE::UserEvents::GetSingleton()->wait);
        button->GetRuntimeData().value = 1.0f;
        button->GetRuntimeData().heldDownSecs = 0.0f;
        RE::InputEvent* event = button;
        controls->ProcessEvent(&event, source);
        button->GetRuntimeData().value = 0.0f;
        button->GetRuntimeData().heldDownSecs = 0.01f;
        controls->ProcessEvent(&event, source);
        button->GetRuntimeData() = savedData;
        button->SetUserEvent(savedUserEvent);
        button->next = savedNext;
    }

    class MenuSink final : public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
        RE::BSEventNotifyControl ProcessEvent(
            const RE::MenuOpenCloseEvent* event,
            RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override
        {
            if (event) {
                SkyPromptClient::RefreshMenuVisibility();
                if (event->opening && !GameplayAvailable()) {
                    FlightInput::Cancel();
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };
    MenuSink menuSink;
}

void FlightInput::Install()
{
    RE::UI::GetSingleton()->AddEventSink<RE::MenuOpenCloseEvent>(&menuSink);
}

void FlightInput::Cancel()
{
    pendingTaps.clear();
    for (auto& [button, press] : consumedButtons) {
        press.action.clear();
    }
    backUsed = true;
    backArmed = false;
    UpdateVerticalInput();
}

void FlightInput::Reset()
{
    Cancel();
    consumedButtons.clear();
    backHeld = false;
    backUsed = false;
}

void FlightInput::Update()
{
    if (!GameplayAvailable() || !PromptManager::canFly || !PromptManager::isVisible) {
        Cancel();
        return;
    }
    FlushExpiredTaps();
    for (auto& [button, press] : consumedButtons) {
        if (!press.action.empty() && !ActionAvailable(press.action)) {
            press.action.clear();
        }
    }
    UpdateVerticalInput();
}

bool FlightInput::OnInput(RE::InputEvent* event)
{
    if (!event) {
        return false;
    }
    if (event->GetEventType() == RE::INPUT_EVENT_TYPE::kDeviceConnect) {
        Reset();
        return false;
    }
    Update();
    auto button = event->AsButtonEvent();
    if (!button) {
        return false;
    }
    const Button key{ event->GetDevice(), button->GetIDCode() };

    // Drain captured releases even after a menu or flight-state change. Never
    // let the tail of a consumed chord become a vanilla action.
    if (auto it = consumedButtons.find(key); it != consumedButtons.end()) {
        if (!button->IsDown()) {
            if (button->IsUp()) {
                if (auto pending = pendingTaps.find(key); pending != pendingTaps.end()) {
                    pending->second.released = true;
                }
                consumedButtons.erase(it);
                UpdateVerticalInput();
            } else if (it->second.pendingHold && !it->second.action.empty() &&
                button->GetRuntimeData().heldDownSecs >= hiddenPromptHoldSeconds) {
                const auto action = it->second.action;
                it->second.pendingHold = false;
                if (GameplayAvailable() && ActionAvailable(action)) {
                    Activate(action);
                }
            }
            return true;
        }
        consumedButtons.erase(it); // A fresh press also recovers a lost release.
        UpdateVerticalInput();
    }

    if (key == Button{ gamepad, backKey }) {
        if (backHeld && !button->IsDown()) {
            if (button->IsUp()) {
                const bool replay = !backUsed && GameplayAvailable();
                backHeld = false;
                backArmed = false;
                std::erase_if(pendingTaps, [](const auto& entry) { return entry.second.modified; });
                for (auto& [heldButton, press] : consumedButtons) {
                    if (press.modified) {
                        press.action.clear();
                    }
                }
                UpdateVerticalInput();
                if (replay) {
                    ReplayWaitTap(button);
                }
            }
            return true;
        }
        if (button->IsDown() && GameplayAvailable() && PromptManager::canFly &&
            PromptManager::isVisible && UsesBackModifier()) {
            backHeld = true;
            backUsed = false;
            backArmed = true;
            return true;
        }
        // An unmodified Back binding in a legacy configuration still works
        // when no action uses Back as a modifier.
        if (UsesBackModifier()) {
            return false;
        }
    }

    if (!GameplayAvailable() || !button->IsDown()) {
        return false;
    }
    for (const auto action : actions) {
        // Hidden SkyPrompt visuals must not remove keyboard/mouse actions.
        // Capture those holds locally; visible prompts retain SkyPrompt handling.
        const bool promptAction = key.first != gamepad &&
            std::string_view(action).starts_with("skyprompt.");
        if (promptAction && ::Configuration::Get().visuals.showHints) {
            continue;
        }
        if (!ActionAvailable(action)) {
            continue;
        }
        for (const auto& binding : InputConfig::GetBindings(action)) {
            if (binding.device != key.first || binding.key != key.second ||
                (binding.modifier && (!backHeld || !backArmed))) {
                continue;
            }
            if (binding.doublePress) {
                if (backHeld) {
                    backUsed = true;
                }
                ProcessDoublePress(key, action, binding);
                return true;
            }
            consumedButtons[key] = { action, binding.modifier.has_value(), promptAction };
            if (backHeld) {
                backUsed = true;
            }
            if (!promptAction) {
                Activate(action);
            }
            return true;
        }
    }
    return false;
}
