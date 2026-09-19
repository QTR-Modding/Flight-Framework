#include "Localization.h"

#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <string_view>

#include <nlohmann/json.hpp>

#include "PCH.h"

namespace {
    using json = nlohmann::json;
    struct TransparentStringHash {
        using is_transparent = void;

        std::size_t operator()(std::string_view value) const noexcept
        {
            return std::hash<std::string_view> {}(value);
        }
    };

    using TranslationMap = std::unordered_map<std::string, std::string, TransparentStringHash, std::equal_to<>>;

    std::filesystem::path GetTranslationsPath()
    {
        return std::filesystem::path("Data") / "SKSE" / "Plugins" / "FlightFrameworkStrings.json";
    }

    TranslationMap g_translations = {
        { "controls.back_view", "Back/View" },
        { "skyprompt.fall", "Fall" },
        { "ui.sections.debug", "Debug" },
        { "ui.sections.configuration", "Configuration" },
        { "ui.sections.visuals", "Visuals" },
        { "ui.visuals.description", "Changes apply immediately. Flight controls remain available with visuals hidden." },
        { "ui.visuals.show_speed_bar", "Show speed bar" },
        { "ui.visuals.show_hints", "Show on-screen hints" },
        { "ui.configuration.description", "Edit the flight values below, then press Save to persist them." },
        { "ui.actions.save", "Save" },
        { "ui.actions.add_helmet", "Add Helmet" },
        { "ui.status.save_prompt", "Changes are only written to disk when Save is pressed." },
        { "ui.status.save_success", "Saved configuration to Data/SKSE/Plugins/FlightFramework.json." },
        { "ui.status.save_failure", "Failed to save configuration. Check the log for details." },
        { "ui.config.headers.main_flight_state", "Main Flight State" },
        { "ui.config.headers.take_off_transition_state", "Take Off Transition State" },
        { "ui.config.headers.controlled_fall_state", "Controlled Fall State" },
        { "ui.config.main_flight_state.default_max_speed", "[Speed] Default Max Speed" },
        { "ui.config.main_flight_state.sprint_max_speed", "[Speed] Sprint Max Speed" },
        { "ui.config.main_flight_state.regular_max_speed", "[Speed] Regular Max Speed" },
        { "ui.config.main_flight_state.input_damping", "[Speed] Input Damping" },
        { "ui.config.main_flight_state.time_to_reach_max_speed", "[Speed] Time To Reach Max Speed" },
        { "ui.config.main_flight_state.damage_multiplier", "[Collision] Damage Multiplier" },
        { "ui.config.take_off_transition_state.body_max_speed", "[Speed] Body Max Speed" },
        { "ui.config.take_off_transition_state.body_time_to_reach_max_speed", "[Speed] Body Time To Reach Max Speed" },
        { "ui.config.take_off_transition_state.body_damping", "[Speed] Body Damping" },
        { "ui.config.take_off_transition_state.transition_velocity_threshold", "[Speed] Transition Velocity Threshold" },
        { "ui.config.controlled_fall_state.initial_speed", "[Speed] Initial Speed" },
        { "ui.config.controlled_fall_state.input_damping", "[Speed] Input Damping" },
        { "ui.config.controlled_fall_state.max_speed", "[Speed] Max Speed" },
        { "ui.config.controlled_fall_state.time_to_reach_max_speed", "[Speed] Time To Reach Max Speed" },
        { "ui.config.controlled_fall_state.forward_velocity", "[Speed] Forward Velocity" },
    };
}

void Localization::LoadTranslations()
{
    const auto path = GetTranslationsPath();

    if (!std::filesystem::exists(path)) {
        logger::warn("FlightFramework strings not found at {}. Using embedded defaults.", path.string());
        return;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        logger::error("Failed to open FlightFramework strings at {}", path.string());
        return;
    }

    try {
        json root;
        file >> root;

        if (!root.is_object()) {
            logger::error("FlightFramework strings at {} must be a JSON object", path.string());
            return;
        }

        for (const auto& [key, value] : root.items()) {
            if (!value.is_string()) {
                logger::warn("Ignoring non-string translation key '{}' from {}", key, path.string());
                continue;
            }

            g_translations[key] = value.get<std::string>();
        }

        logger::info("Loaded FlightFramework strings from {}", path.string());
    } catch (const std::exception& e) {
        logger::error("Failed to parse FlightFramework strings at {}: {}", path.string(), e.what());
    }
}

const std::string& Localization::GetTranslation(std::string_view key)
{
    static const std::string empty;

    if (const auto it = g_translations.find(key); it != g_translations.end()) {
        return it->second;
    }

    return empty;
}
