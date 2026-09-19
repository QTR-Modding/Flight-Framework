#include "UI.h"

#include <array>
#include <cstdio>
#include <format>
#include <string>
#include <vector>

#include "Bar.h"
#include "FlightManager.h"
#include "FlightInput.h"
#include "Localization.h"
#include "Persistence.h"
#include "InputConfig.h"
#include "PromptManager.h"
#include "SkyPromptClient.h"
#include "Utils.h"
namespace {
    struct BindingEditorRow {
        RE::INPUT_DEVICE device;
        std::string keyName = "";
        std::optional<std::string> modifier;
        bool doublePress = false;
    };

    struct BindingEditorAction {
        std::string action;
        std::vector<BindingEditorRow> bindings;
    };

    void RenderFloatProperty(const char* translationKey, float& value)
    {
        const auto label = std::format("{}##{}", Localization::GetTranslation(translationKey), translationKey);
        ImGuiMCP::InputFloat(
            label.c_str(),
            &value,
            0.1f,
            1.0f,
            "%.3f",
            ImGuiMCP::ImGuiInputTextFlags_CharsScientific);
    }

    void RenderMainFlightStateConfig(::Configuration::MainFlightStateConfig& config)
    {
        if (!ImGuiMCP::CollapsingHeader(
                Localization::GetTranslation("ui.config.headers.main_flight_state").c_str(),
                ImGuiMCP::ImGuiTreeNodeFlags_DefaultOpen)) {
            return;
        }

        RenderFloatProperty("ui.config.main_flight_state.default_max_speed", config.defaultMaxSpeed);
        RenderFloatProperty("ui.config.main_flight_state.sprint_max_speed", config.sprintMaxSpeed);
        RenderFloatProperty("ui.config.main_flight_state.regular_max_speed", config.regularMaxSpeed);
        RenderFloatProperty("ui.config.main_flight_state.input_damping", config.inputDamping);
        RenderFloatProperty("ui.config.main_flight_state.time_to_reach_max_speed", config.timeToReachMaxSpeed);
        RenderFloatProperty("ui.config.main_flight_state.damage_multiplier", config.damageMultiplier);
    }

    void RenderTakeOffTransitionStateConfig(::Configuration::TakeOffTransitionStateConfig& config)
    {
        if (!ImGuiMCP::CollapsingHeader(
                Localization::GetTranslation("ui.config.headers.take_off_transition_state").c_str(),
                ImGuiMCP::ImGuiTreeNodeFlags_DefaultOpen)) {
            return;
        }

        RenderFloatProperty("ui.config.take_off_transition_state.body_max_speed", config.bodyMaxSpeed);
        RenderFloatProperty("ui.config.take_off_transition_state.body_time_to_reach_max_speed", config.bodyTimeToReachMaxSpeed);
        RenderFloatProperty("ui.config.take_off_transition_state.body_damping", config.bodyDamping);
        RenderFloatProperty("ui.config.take_off_transition_state.transition_velocity_threshold", config.transitionVelocityThreshold);
    }

    void RenderControlledFallStateConfig(::Configuration::ControlledFallStateConfig& config)
    {
        if (!ImGuiMCP::CollapsingHeader(
                Localization::GetTranslation("ui.config.headers.controlled_fall_state").c_str(),
                ImGuiMCP::ImGuiTreeNodeFlags_DefaultOpen)) {
            return;
        }

        RenderFloatProperty("ui.config.controlled_fall_state.initial_speed", config.initialSpeed);
        RenderFloatProperty("ui.config.controlled_fall_state.input_damping", config.inputDamping);
        RenderFloatProperty("ui.config.controlled_fall_state.max_speed", config.maxSpeed);
        RenderFloatProperty("ui.config.controlled_fall_state.time_to_reach_max_speed", config.timeToReachMaxSpeed);
        RenderFloatProperty("ui.config.controlled_fall_state.forward_velocity", config.forwardVelocity);
    }

    std::vector<BindingEditorAction> LoadBindingEditorState()
    {
        std::vector<BindingEditorAction> state;

        for (const auto& actionBindings : InputConfig::GetEditableBindings()) {
            BindingEditorAction actionState;
            actionState.action = actionBindings.action;
            actionState.bindings.reserve(actionBindings.bindings.size());

            for (const auto& binding : actionBindings.bindings) {
                BindingEditorRow row;
                row.device = binding.device;
                row.keyName = binding.keyName;
                row.modifier = binding.modifier;
                row.doublePress = binding.doublePress;
                actionState.bindings.push_back(row);
            }

            state.push_back(std::move(actionState));
        }

        return state;
    }

    std::vector<ActionBindings> BuildBindingConfig(const std::vector<BindingEditorAction>& editorState)
    {
        std::vector<ActionBindings> bindings;
        bindings.reserve(editorState.size());

        for (const auto& actionState : editorState) {
            ActionBindings actionBindings;
            actionBindings.action = actionState.action;
            actionBindings.bindings.reserve(actionState.bindings.size());

            for (const auto& row : actionState.bindings) {
                actionBindings.bindings.push_back({
                    .device = row.device,
                    .keyName = row.keyName,
                    .modifier = row.modifier,
                    .doublePress = row.doublePress,
                });
            }

            bindings.push_back(std::move(actionBindings));
        }

        return bindings;
    }

    const std::string& GetBindingLabel(const std::string& action)
    {
        const auto& translated = Localization::GetTranslation(action);
        if (!translated.empty()) {
            return translated;
        }

        return action;
    }
}

void UI::Register()
{
    if (!SKSEMenuFramework::IsInstalled()) {
        return;
    }

    menuFramework = GetModuleHandle(L"SKSEMenuFramework");
    if (!menuFramework) {
        logger::error("Cannot register Flight Framework UI: SKSE Menu Framework is not loaded.");
        return;
    }

    SKSEMenuFramework::SetSection(MOD_NAME);
#ifndef NDEBUG
    SKSEMenuFramework::AddSectionItem(Localization::GetTranslation("ui.sections.debug").c_str(), Main::Render);
#endif  // !NDEBUG
    SKSEMenuFramework::AddSectionItem(Localization::GetTranslation("ui.sections.configuration").c_str(), Configuration::Render);
    SKSEMenuFramework::AddSectionItem("Key Bindings", KeyBindings::Render);
    SKSEMenuFramework::AddSectionItem(Localization::GetTranslation("ui.sections.visuals").c_str(), Visuals::Render);
    SKSEMenuFramework::AddInputEvent(Main::OnInput);
    SKSEMenuFramework::AddHudElement(Main::Hud);
}

void TeleportUp()
{
    auto player = RE::PlayerCharacter::GetSingleton();
    player->SetPosition(player->GetPosition() + RE::NiPoint3(0, 0, 10000), true);
}

void __stdcall UI::Main::Hud()
{
    SkyPromptClient::RefreshMenuVisibility();
    if (Utils::IsMenuOpen()) {
        return;
    }

    if (!::Configuration::Get().visuals.showSpeedBar) {
        return;
    }

    auto state = FlightManager::GetCurrentState();
    if (!state) {
        return;
    }


    

    ImGuiMCP::ImGuiIO* io = ImGuiMCP::GetIO();

    float screenW = io->DisplaySize.x;
    float screenH = io->DisplaySize.y;

    float barWidth = screenW * 0.20f;
    float barHeight = screenH * 0.022f;
    float rounding = barHeight * 0.1f;
    float verticalOffset = screenH * 0.10f;

    ImGuiMCP::ImVec2 pos((screenW - barWidth) * 0.5f, screenH - verticalOffset - barHeight);
    ImGuiMCP::ImVec2 size(barWidth, barHeight);

    float speed = state->GetSpeed();
    float maxSpeed = state->GetMaxSpeed();

    float value = maxSpeed != 0.0f ? std::abs(speed) / std::abs(maxSpeed) : 0.0f;

    auto rawScreenSize = RE::BSGraphics::Renderer::GetScreenSize();
    RE::NiPoint2 screenSize = { static_cast<float>(rawScreenSize.width), static_cast<float>(rawScreenSize.height) };
    float sizeRatio = screenSize.x / 1920;

    Bar::Render(sizeRatio, value, Bar::BarPosition::BottomCenter, 100 * sizeRatio);
}

void __stdcall UI::Configuration::Render()
{
    auto& config = ::Configuration::Get();
    static std::string saveStatus = Localization::GetTranslation("ui.status.save_prompt");

    ImGuiMCP::TextUnformatted(Localization::GetTranslation("ui.configuration.description").c_str());
    ImGuiMCP::Spacing();

    RenderMainFlightStateConfig(config.mainFlightState);
    RenderTakeOffTransitionStateConfig(config.takeOffTransitionState);
    RenderControlledFallStateConfig(config.controlledFallState);

    ImGuiMCP::Spacing();
    if (ImGuiMCP::Button(Localization::GetTranslation("ui.actions.save").c_str())) {
        saveStatus = Persistence::Save()
            ? Localization::GetTranslation("ui.status.save_success")
            : Localization::GetTranslation("ui.status.save_failure");
    }

    ImGuiMCP::SameLine();
    ImGuiMCP::TextUnformatted(saveStatus.c_str());
}

void __stdcall UI::Visuals::Render()
{
    auto& config = ::Configuration::Get().visuals;
    static std::string saveStatus = Localization::GetTranslation("ui.status.save_prompt");

    ImGuiMCP::TextUnformatted(Localization::GetTranslation("ui.visuals.description").c_str());
    ImGuiMCP::Spacing();
    ImGuiMCP::Checkbox(Localization::GetTranslation("ui.visuals.show_speed_bar").c_str(), &config.showSpeedBar);
    if (ImGuiMCP::Checkbox(Localization::GetTranslation("ui.visuals.show_hints").c_str(), &config.showHints)) {
        SKSE::GetTaskInterface()->AddTask([] { SkyPromptClient::RefreshBindings(); });
    }

    ImGuiMCP::Spacing();
    if (ImGuiMCP::Button(Localization::GetTranslation("ui.actions.save").c_str())) {
        saveStatus = Persistence::Save()
            ? Localization::GetTranslation("ui.status.save_success")
            : Localization::GetTranslation("ui.status.save_failure");
    }
    ImGuiMCP::SameLine();
    ImGuiMCP::TextUnformatted(saveStatus.c_str());
}

void __stdcall UI::KeyBindings::Render()
{
    static std::vector<BindingEditorAction> editorState = LoadBindingEditorState();
    static std::string saveStatus = Localization::GetTranslation("ui.status.save_prompt");

    if (editorState.empty() && !InputConfig::GetEditableBindings().empty()) {
        editorState = LoadBindingEditorState();
    }

    ImGuiMCP::TextUnformatted(Localization::GetTranslation("ui.configuration.editKeyBinding").c_str());
    ImGuiMCP::Spacing();

    for (auto& actionState : editorState) {
        ImGuiMCP::PushID(actionState.action.c_str());
        if (ImGuiMCP::CollapsingHeader(GetBindingLabel(actionState.action).c_str(), ImGuiMCP::ImGuiTreeNodeFlags_DefaultOpen)) {
            for (auto& row : actionState.bindings) {

                const char** keys = nullptr;
                std::size_t keyCount = 0;
                std::string deviceName = "";

                if (row.device == RE::INPUT_DEVICE::kKeyboard) {
                    deviceName = Localization::GetTranslation("device.keyboard");
                    keys = InputConfig::keyboardKeys;
                    keyCount = InputConfig::keyboardKeysLength;
                } else if (row.device == RE::INPUT_DEVICE::kGamepad) {
                    deviceName = Localization::GetTranslation("device.gamepad");
                    keys = InputConfig::gamepadKeys;
                    keyCount = InputConfig::gamepadKeysLength;
                } else {
                    deviceName = Localization::GetTranslation("device.mouse");
                    keys = InputConfig::mouseKeys;
                    keyCount = InputConfig::mouseKeysLength;
                }

                int currentItem = 0;
                for (int i = 0; i < keyCount; ++i) {
                    if (keys[i] == row.keyName) {
                        currentItem = i;
                        break;
                    }
                }
                ImGuiMCP::Text(deviceName.c_str());
                if (ImGuiMCP::Combo(std::format("##{}-{}-{}", actionState.action, row.keyName, (int)row.device).c_str(), &currentItem, keys, keyCount)) {
                    row.keyName = keys[currentItem];
                }
                if (row.device == RE::INPUT_DEVICE::kGamepad) {
                    if (actionState.action == "skyprompt.fly" || actionState.action == "skyprompt.land" ||
                        actionState.action == "skyprompt.fall") {
                        ImGuiMCP::Checkbox(
                            std::format("Double press##{}-{}", actionState.action, (int)row.device).c_str(),
                            &row.doublePress);
                    }
                    const char* modifiers[] = { "None", "Back / View (hold)" };
                    int selectedModifier = row.modifier ? 1 : 0;
                    if (ImGuiMCP::Combo(
                            std::format("Modifier##{}-{}", actionState.action, (int)row.device).c_str(),
                            &selectedModifier, modifiers, 2)) {
                        row.modifier = selectedModifier == 1
                            ? std::optional<std::string>("back") : std::nullopt;
                    }
                }
            }
        }
        ImGuiMCP::PopID();
    }

    ImGuiMCP::Spacing();
    if (ImGuiMCP::Button(Localization::GetTranslation("ui.actions.save").c_str())) {
        std::string errorMessage;
        if (InputConfig::SaveEditableBindings(BuildBindingConfig(editorState), errorMessage)) {
            SKSE::GetTaskInterface()->AddTask([] { FlightInput::Cancel(); });
            SkyPromptClient::RefreshBindings();
            editorState = LoadBindingEditorState();
            saveStatus = Localization::GetTranslation("ui.status.save_success_keybindings");
        } else {
            saveStatus = errorMessage.empty() ? Localization::GetTranslation("ui.status.save_failure_keybindings") : errorMessage;
        }
    }

    ImGuiMCP::SameLine();
    ImGuiMCP::TextUnformatted(saveStatus.c_str());
}

void __stdcall UI::Main::Render()
{
    if (ImGuiMCP::Button(Localization::GetTranslation("ui.actions.add_helmet").c_str())) {
        auto id = RE::TESDataHandler::GetSingleton()->LookupFormID(0x801, "GlassHelmentOfFlight.esp");
        auto bound = RE::TESForm::LookupByID<RE::TESBoundObject>(id);
        RE::PlayerCharacter::GetSingleton()->AddObjectToContainer(bound, nullptr, 1, nullptr);
    }
}

bool _stdcall UI::Main::OnInput(RE::InputEvent* event)
{
    if (!event) {
        return false;
    }
    if (event->GetDevice() == RE::INPUT_DEVICE::kKeyboard || event->GetDevice() == RE::INPUT_DEVICE::kGamepad) {
        if (lastDevice != event->GetDevice()) {
            lastDevice = event->GetDevice();
            SkyPromptClient::SetInputDevice(lastDevice);
        }
    }
    const bool consumed = FlightInput::OnInput(event);
    return consumed || FlightManager::OnInput(event);
}
