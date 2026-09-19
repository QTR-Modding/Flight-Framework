#include "Persistence.h"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "Configuration.h"

using json = nlohmann::json;

namespace {
    std::filesystem::path GetConfigPath()
    {
        return std::filesystem::path("Data") / "SKSE" / "Plugins" / "FlightFramework.json";
    }

    template <class T>
    void AssignIfPresent(const json& section, const char* key, T& target)
    {
        if (const auto it = section.find(key); it != section.end() && !it->is_null()) {
            target = it->get<T>();
        }
    }

    json SerializeMainFlightStateConfig(const ::Configuration::MainFlightStateConfig& config)
    {
        return json{
            { "defaultMaxSpeed", config.defaultMaxSpeed },
            { "sprintMaxSpeed", config.sprintMaxSpeed },
            { "regularMaxSpeed", config.regularMaxSpeed },
            { "inputDamping", config.inputDamping },
            { "timeToReachMaxSpeed", config.timeToReachMaxSpeed },
            { "damageMultiplier", config.damageMultiplier },
            { "initialDirectionalMaxSpeed", config.initialDirectionalMaxSpeed },
        };
    }

    json SerializeTakeOffTransitionStateConfig(const ::Configuration::TakeOffTransitionStateConfig& config)
    {
        return json{
            { "bodyMaxSpeed", config.bodyMaxSpeed },
            { "bodyTimeToReachMaxSpeed", config.bodyTimeToReachMaxSpeed },
            { "bodyDamping", config.bodyDamping },
            { "transitionVelocityThreshold", config.transitionVelocityThreshold },
            { "groundRaycastDistance", config.groundRaycastDistance },
            { "minHeightAboveGround", config.minHeightAboveGround },
            { "damageMultiplier", config.damageMultiplier },
        };
    }

    json SerializeControlledFallStateConfig(const ::Configuration::ControlledFallStateConfig& config)
    {
        return json{
            { "initialSpeed", config.initialSpeed },
            { "inputDamping", config.inputDamping },
            { "maxSpeed", config.maxSpeed },
            { "timeToReachMaxSpeed", config.timeToReachMaxSpeed },
            { "forwardVelocity", config.forwardVelocity },
            { "damageMultiplier", config.damageMultiplier },
        };
    }

    json SerializeVisualsConfig(const ::Configuration::VisualsConfig& config)
    {
        return json{
            { "showSpeedBar", config.showSpeedBar },
            { "showHints", config.showHints },
        };
    }

    void LoadVisualsConfig(const json& root)
    {
        if (!root.contains("Visuals") || !root.at("Visuals").is_object()) {
            return;
        }

        const auto& section = root.at("Visuals");
        auto& config = ::Configuration::Get().visuals;
        AssignIfPresent(section, "showSpeedBar", config.showSpeedBar);
        AssignIfPresent(section, "showHints", config.showHints);
    }

    json BuildConfigRoot(const ::Configuration::Config& config)
    {
        return json{
            { "Visuals", SerializeVisualsConfig(config.visuals) },
            { "MainFlightState", SerializeMainFlightStateConfig(config.mainFlightState) },
            { "TakeOffTransitionState", SerializeTakeOffTransitionStateConfig(config.takeOffTransitionState) },
            { "ControlledFallState", SerializeControlledFallStateConfig(config.controlledFallState) },
        };
    }

    void LoadMainFlightStateConfig(const json& root)
    {
        if (!root.contains("MainFlightState") || !root.at("MainFlightState").is_object()) {
            return;
        }

        const auto& section = root.at("MainFlightState");
        auto& config = ::Configuration::Get().mainFlightState;
        AssignIfPresent(section, "defaultMaxSpeed", config.defaultMaxSpeed);
        AssignIfPresent(section, "sprintMaxSpeed", config.sprintMaxSpeed);
        AssignIfPresent(section, "regularMaxSpeed", config.regularMaxSpeed);
        AssignIfPresent(section, "inputDamping", config.inputDamping);
        AssignIfPresent(section, "timeToReachMaxSpeed", config.timeToReachMaxSpeed);
        AssignIfPresent(section, "damageMultiplier", config.damageMultiplier);
        AssignIfPresent(section, "initialDirectionalMaxSpeed", config.initialDirectionalMaxSpeed);
    }

    void LoadTakeOffTransitionStateConfig(const json& root)
    {
        if (!root.contains("TakeOffTransitionState") || !root.at("TakeOffTransitionState").is_object()) {
            return;
        }

        const auto& section = root.at("TakeOffTransitionState");
        auto& config = ::Configuration::Get().takeOffTransitionState;
        AssignIfPresent(section, "bodyMaxSpeed", config.bodyMaxSpeed);
        AssignIfPresent(section, "bodyTimeToReachMaxSpeed", config.bodyTimeToReachMaxSpeed);
        AssignIfPresent(section, "bodyDamping", config.bodyDamping);
        AssignIfPresent(section, "transitionVelocityThreshold", config.transitionVelocityThreshold);
        AssignIfPresent(section, "groundRaycastDistance", config.groundRaycastDistance);
        AssignIfPresent(section, "minHeightAboveGround", config.minHeightAboveGround);
        AssignIfPresent(section, "damageMultiplier", config.damageMultiplier);
    }

    void LoadControlledFallStateConfig(const json& root)
    {
        if (!root.contains("ControlledFallState") || !root.at("ControlledFallState").is_object()) {
            return;
        }

        const auto& section = root.at("ControlledFallState");
        auto& config = ::Configuration::Get().controlledFallState;
        AssignIfPresent(section, "initialSpeed", config.initialSpeed);
        AssignIfPresent(section, "inputDamping", config.inputDamping);
        AssignIfPresent(section, "maxSpeed", config.maxSpeed);
        AssignIfPresent(section, "timeToReachMaxSpeed", config.timeToReachMaxSpeed);
        AssignIfPresent(section, "forwardVelocity", config.forwardVelocity);
        AssignIfPresent(section, "damageMultiplier", config.damageMultiplier);
    }
}

void Persistence::Install()
{
    const auto path = GetConfigPath();

    if (!std::filesystem::exists(path)) {
        logger::warn("FlightFramework config not found at {}. Using defaults.", path.string());
        return;
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        logger::error("Failed to open FlightFramework config at {}", path.string());
        return;
    }

    try {
        json root;
        file >> root;
        LoadVisualsConfig(root);
        LoadMainFlightStateConfig(root);
        LoadTakeOffTransitionStateConfig(root);
        LoadControlledFallStateConfig(root);
        logger::info("Loaded FlightFramework config from {}", path.string());
    } catch (const std::exception& e) {
        logger::error("Failed to parse FlightFramework config at {}: {}", path.string(), e.what());
    }
}

bool Persistence::Save()
{
    const auto path = GetConfigPath();

    try {
        std::filesystem::create_directories(path.parent_path());

        std::ofstream file(path);
        if (!file.is_open()) {
            logger::error("Failed to open FlightFramework config for writing at {}", path.string());
            return false;
        }

        file << BuildConfigRoot(::Configuration::Get()).dump(4) << '\n';
        if (!file.good()) {
            logger::error("Failed to write FlightFramework config to {}", path.string());
            return false;
        }

        logger::info("Saved FlightFramework config to {}", path.string());
        return true;
    } catch (const std::exception& e) {
        logger::error("Failed to save FlightFramework config at {}: {}", path.string(), e.what());
        return false;
    }
}
