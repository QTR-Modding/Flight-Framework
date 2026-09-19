#include "SkyPromptAPI.h"
#include "SkyPromptClient.h"

#include <chrono>
#include <deque>
#include <format>
#include <string>
#include <utility>
#include <vector>

#include "Configuration.h"
#include "InputConfig.h"
#include "Localization.h"
#include "Menu.h"
#include "PromptManager.h"
#include "SKSEMenuFramework.h"

namespace {
    using PromptButton = std::pair<RE::INPUT_DEVICE, SkyPromptAPI::ButtonID>;
    using PromptClock = std::chrono::steady_clock;

    constexpr std::string_view flightFrameworkTheme = "FlightFramework";
    constexpr auto promptRetryDelay = std::chrono::seconds(1);

    enum class PromptAction : SkyPromptAPI::ActionID {
        kNone,
        kFly,
        kLand,
        kFall,
        kFlyUp,
        kFlyDown,
        kSprint,
    };

    enum class PromptDisplayState {
        kHidden,
        kWalking,
        kFlying,
    };

    struct PromptSpec {
        std::string text;
        SkyPromptAPI::EventID eventID;
        PromptAction action;
        SkyPromptAPI::PromptType type;
        PromptButton button;
    };

    class PromptEntrySink;
    void HandlePromptEvent(const PromptEntrySink& sink, SkyPromptAPI::PromptEvent event);

    class PromptEntrySink final : public SkyPromptAPI::PromptSink {
    public:
        explicit PromptEntrySink(PromptSpec spec)
            : text(std::move(spec.text)),
              button(spec.button),
              prompt(
                  text,
                  spec.eventID,
                  static_cast<SkyPromptAPI::ActionID>(spec.action),
                  spec.type,
                  0,
                  std::span<const PromptButton>(&button, 1),
                  0xFFFFFFFF),
              action(spec.action),
              nextRetry(PromptClock::now())
        {}

        PromptEntrySink(const PromptEntrySink&) = delete;
        PromptEntrySink& operator=(const PromptEntrySink&) = delete;
        PromptEntrySink(PromptEntrySink&&) = delete;
        PromptEntrySink& operator=(PromptEntrySink&&) = delete;
        ~PromptEntrySink() override = default;

        void ProcessEvent(const SkyPromptAPI::PromptEvent event) const override
        {
            HandlePromptEvent(*this, event);
        }

        std::span<const SkyPromptAPI::Prompt> GetPrompts() const override
        {
            return std::span<const SkyPromptAPI::Prompt>(&prompt, 1);
        }

        [[nodiscard]] PromptAction GetAction() const
        {
            return action;
        }

        [[nodiscard]] const std::string& GetText() const
        {
            return text;
        }

        [[nodiscard]] bool IsQueued() const
        {
            return queued;
        }

        [[nodiscard]] bool CanRetry(PromptClock::time_point now) const
        {
            return now >= nextRetry;
        }

        void MarkQueued() const
        {
            queued = true;
            retryLogged = false;
        }

        void MarkRemoved() const
        {
            queued = false;
            retryLogged = false;
            nextRetry = PromptClock::now();
        }

        void MarkTimedOut() const
        {
            queued = false;
            retryLogged = false;
            nextRetry = PromptClock::now() + promptRetryDelay;
        }

        void MarkFailed(PromptClock::time_point now) const
        {
            queued = false;
            nextRetry = now + promptRetryDelay;
            if (!retryLogged) {
                logger::warn("SkyPrompt could not queue Flight Framework '{}' prompt; waiting to retry.", text);
                retryLogged = true;
            }
        }

    private:
        std::string text;
        PromptButton button;
        SkyPromptAPI::Prompt prompt;
        PromptAction action;
        mutable bool queued = false;
        mutable bool retryLogged = false;
        mutable PromptClock::time_point nextRetry;
    };

    using PromptEntries = std::deque<PromptEntrySink>;

    RE::INPUT_DEVICE activeDevice = RE::INPUT_DEVICE::kKeyboard;
    bool promptsBlockedByMenu = false;
    SkyPromptAPI::ClientID clientID = 0;
    PromptDisplayState desiredPromptState = PromptDisplayState::kHidden;
    PromptEntries walkingPrompts;
    PromptEntries flyingPrompts;

    bool IsPromptBlockedByMenu()
    {
        return Menu::IsOpen() || SKSEMenuFramework::IsAnyBlockingWindowOpened();
    }

    bool UsesController()
    {
        return activeDevice == RE::INPUT_DEVICE::kGamepad;
    }

    bool MatchesActiveDevice(RE::INPUT_DEVICE device)
    {
        if (UsesController()) {
            return device == RE::INPUT_DEVICE::kGamepad;
        }
        return device == RE::INPUT_DEVICE::kKeyboard || device == RE::INPUT_DEVICE::kMouse;
    }

    std::string GetPromptText(const std::string& action, const ResolvedInputBinding& binding)
    {
        std::string text = Localization::GetTranslation(action);
        if (binding.modifier) {
            text = std::format("{} + {}", Localization::GetTranslation("controls.back_view"), text);
        }
        if (binding.doublePress) {
            text += " (2x)";
        }
        return text;
    }

    void AddActionPrompts(
        std::vector<PromptSpec>& specs,
        SkyPromptAPI::EventID& nextEventID,
        const std::string& action,
        PromptAction promptAction,
        SkyPromptAPI::PromptType type)
    {
        for (const auto& binding : InputConfig::GetBindings(action)) {
            if (!MatchesActiveDevice(binding.device)) {
                continue;
            }
            specs.push_back({
                GetPromptText(action, binding),
                nextEventID++,
                promptAction,
                type,
                { binding.device, binding.key },
            });
        }
    }

    void AddSprintPrompt(std::vector<PromptSpec>& specs, SkyPromptAPI::EventID& nextEventID)
    {
        const auto controlMap = RE::ControlMap::GetSingleton();
        const auto userEvents = RE::UserEvents::GetSingleton();
        if (!controlMap || !userEvents) {
            logger::error("Cannot create the SkyPrompt sprint hint: Skyrim controls are unavailable.");
            return;
        }

        const auto sprintKey = controlMap->GetMappedKey(userEvents->sprint, activeDevice);
        if (sprintKey == RE::ControlMap::kInvalid) {
            logger::warn("Cannot create the SkyPrompt sprint hint: Sprint is not mapped for the active device.");
            return;
        }

        specs.push_back({
            Localization::GetTranslation("controls.sprint"),
            nextEventID++,
            PromptAction::kSprint,
            SkyPromptAPI::PromptType::kHint,
            { activeDevice, sprintKey },
        });
    }

    std::vector<PromptSpec> BuildWalkingPromptSpecs()
    {
        std::vector<PromptSpec> specs;
        SkyPromptAPI::EventID nextEventID = 1;
        AddActionPrompts(
            specs,
            nextEventID,
            "skyprompt.fly",
            PromptAction::kFly,
            UsesController() ? SkyPromptAPI::PromptType::kHint : SkyPromptAPI::PromptType::kHold);
        return specs;
    }

    std::vector<PromptSpec> BuildFlyingPromptSpecs()
    {
        std::vector<PromptSpec> specs;
        SkyPromptAPI::EventID nextEventID = 1;
        const auto stateChangeType = UsesController()
            ? SkyPromptAPI::PromptType::kHint
            : SkyPromptAPI::PromptType::kHold;

        // Preserve this order: essential flight controls get queue priority over Sprint.
        AddActionPrompts(specs, nextEventID, "skyprompt.land", PromptAction::kLand, stateChangeType);
        AddActionPrompts(specs, nextEventID, "skyprompt.fall", PromptAction::kFall, stateChangeType);
        AddActionPrompts(specs, nextEventID, "ui.flyUp", PromptAction::kFlyUp, SkyPromptAPI::PromptType::kHint);
        AddActionPrompts(specs, nextEventID, "ui.flyDown", PromptAction::kFlyDown, SkyPromptAPI::PromptType::kHint);
        AddSprintPrompt(specs, nextEventID);
        return specs;
    }

    void RebuildPromptEntries(PromptEntries& entries, std::vector<PromptSpec> specs)
    {
        entries.clear();
        for (auto& spec : specs) {
            entries.emplace_back(std::move(spec));
        }
    }

    PromptEntries* GetPromptEntries(PromptDisplayState state)
    {
        if (state == PromptDisplayState::kWalking) {
            return &walkingPrompts;
        }
        if (state == PromptDisplayState::kFlying) {
            return &flyingPrompts;
        }
        return nullptr;
    }

    const char* GetPromptStateName(PromptDisplayState state)
    {
        return state == PromptDisplayState::kFlying ? "flying" : "walking";
    }

    void RemovePromptEntries(PromptEntries& entries, bool forceRemoval)
    {
        for (auto& entry : entries) {
            if (clientID && (forceRemoval || entry.IsQueued())) {
                SkyPromptAPI::RemovePrompt(&entry, clientID);
            }
            entry.MarkRemoved();
        }
    }

    void RemoveAllPrompts(bool forceRemoval = false)
    {
        RemovePromptEntries(walkingPrompts, forceRemoval);
        RemovePromptEntries(flyingPrompts, forceRemoval);
    }

    std::size_t CountQueuedPrompts(const PromptEntries& entries)
    {
        std::size_t count = 0;
        for (const auto& entry : entries) {
            if (entry.IsQueued()) {
                ++count;
            }
        }
        return count;
    }

    bool CanShowDesiredPrompts()
    {
        const auto entries = GetPromptEntries(desiredPromptState);
        return entries && !entries->empty() && clientID && Configuration::Get().visuals.showHints &&
            PromptManager::canFly && PromptManager::isVisible;
    }

    void ReconcilePromptState()
    {
        promptsBlockedByMenu = IsPromptBlockedByMenu();
        if (desiredPromptState == PromptDisplayState::kHidden || !CanShowDesiredPrompts() ||
            promptsBlockedByMenu) {
            RemoveAllPrompts();
            return;
        }

        PromptEntries* desiredEntries = GetPromptEntries(desiredPromptState);
        if (!desiredEntries) {
            return;
        }

        if (desiredPromptState == PromptDisplayState::kWalking) {
            RemovePromptEntries(flyingPrompts, false);
        } else {
            RemovePromptEntries(walkingPrompts, false);
        }

        const auto now = PromptClock::now();
        std::size_t newlyQueued = 0;
        for (auto& entry : *desiredEntries) {
            if (entry.IsQueued() || !entry.CanRetry(now)) {
                continue;
            }

            if (SkyPromptAPI::SendPrompt(&entry, clientID)) {
                entry.MarkQueued();
                ++newlyQueued;
            } else {
                entry.MarkFailed(now);
            }
        }

        if (newlyQueued) {
            logger::info(
                "SkyPrompt queued {}/{} Flight Framework {} prompts.",
                CountQueuedPrompts(*desiredEntries),
                desiredEntries->size(),
                GetPromptStateName(desiredPromptState));
        }
    }

    void SetDesiredPromptState(PromptDisplayState state)
    {
        if (desiredPromptState != state) {
            RemoveAllPrompts();
            desiredPromptState = state;
        }
        ReconcilePromptState();
    }

    void ShowCurrentState()
    {
        if (!PromptManager::isVisible) {
            SetDesiredPromptState(PromptDisplayState::kHidden);
        } else if (PromptManager::isFlying) {
            SetDesiredPromptState(PromptDisplayState::kFlying);
        } else {
            SetDesiredPromptState(PromptDisplayState::kWalking);
        }
    }

    void HandlePromptEvent(const PromptEntrySink& sink, SkyPromptAPI::PromptEvent event)
    {
        if (event.type == SkyPromptAPI::PromptEventType::kTimeout) {
            sink.MarkTimedOut();
            return;
        }
        if (event.type != SkyPromptAPI::PromptEventType::kAccepted ||
            event.prompt.type == SkyPromptAPI::PromptType::kHint ||
            !Configuration::Get().visuals.showHints || !PromptManager::canFly ||
            !PromptManager::isVisible || IsPromptBlockedByMenu()) {
            return;
        }

        const auto action = sink.GetAction();
        if (action == PromptAction::kFly && !PromptManager::isFlying) {
            PromptManager::StartFlying();
        } else if (action == PromptAction::kLand && PromptManager::isFlying) {
            PromptManager::ExitFlight();
        } else if (action == PromptAction::kFall && PromptManager::isFlying) {
            PromptManager::StopFlying();
        }
    }
}

void SkyPromptClient::Install()
{
    menuFramework = GetModuleHandle(L"SKSEMenuFramework");
    if (!menuFramework) {
        logger::warn("SKSE Menu Framework is not loaded; blocking menu detection will be limited.");
    }

    clientID = SkyPromptAPI::RequestClientID();
    if (!clientID) {
        logger::error("SkyPrompt did not provide a client ID. Flight hints will be unavailable.");
    } else if (!SkyPromptAPI::RequestTheme(clientID, flightFrameworkTheme)) {
        logger::warn("SkyPrompt could not load the Flight Framework theme; it may not display every flight hint.");
    }

    RebuildPromptEntries(walkingPrompts, BuildWalkingPromptSpecs());
    RebuildPromptEntries(flyingPrompts, BuildFlyingPromptSpecs());
    promptsBlockedByMenu = IsPromptBlockedByMenu();
    desiredPromptState = PromptDisplayState::kHidden;
}

void SkyPromptClient::PrepareForLoad()
{
    desiredPromptState = PromptDisplayState::kHidden;
    RemoveAllPrompts(true);
}

void SkyPromptClient::Update()
{
    ReconcilePromptState();
}

void SkyPromptClient::ShowFly()
{
    SetDesiredPromptState(PromptDisplayState::kWalking);
}

void SkyPromptClient::ShowExit()
{
    SetDesiredPromptState(PromptDisplayState::kFlying);
}

void SkyPromptClient::Hide()
{
    SetDesiredPromptState(PromptDisplayState::kHidden);
}

void SkyPromptClient::RefreshBindings()
{
    Hide();
    RemoveAllPrompts(true);
    RebuildPromptEntries(walkingPrompts, BuildWalkingPromptSpecs());
    RebuildPromptEntries(flyingPrompts, BuildFlyingPromptSpecs());
    ShowCurrentState();
}

void SkyPromptClient::RefreshMenuVisibility()
{
    Update();
}

void SkyPromptClient::SetInputDevice(RE::INPUT_DEVICE device)
{
    if (device == activeDevice ||
        (device != RE::INPUT_DEVICE::kKeyboard && device != RE::INPUT_DEVICE::kMouse &&
            device != RE::INPUT_DEVICE::kGamepad)) {
        return;
    }

    Hide();
    RemoveAllPrompts(true);
    activeDevice = device;
    RebuildPromptEntries(walkingPrompts, BuildWalkingPromptSpecs());
    RebuildPromptEntries(flyingPrompts, BuildFlyingPromptSpecs());
    ShowCurrentState();
}
