#pragma once

namespace Configuration {
    struct MainFlightStateConfig {
        float defaultMaxSpeed = 30.0f;
        float sprintMaxSpeed = 30.0f;
        float regularMaxSpeed = 10.0f;
        float inputDamping = 0.98f;
        float timeToReachMaxSpeed = 2.0f;
        float damageMultiplier = 1.0f;
        float initialDirectionalMaxSpeed = 50.0f;
    };

    struct TakeOffTransitionStateConfig {
        float bodyMaxSpeed = 50.0f;
        float bodyTimeToReachMaxSpeed = 1.0f;
        float bodyDamping = 0.99f;
        float transitionVelocityThreshold = 1.0f;
        float groundRaycastDistance = 200000.0f;
        float minHeightAboveGround = 100.0f;
        float damageMultiplier = 1.0f;
    };

    struct ControlledFallStateConfig {
        float initialSpeed = 0.0f;
        float inputDamping = 0.98f;
        float maxSpeed = -25.0f;
        float timeToReachMaxSpeed = 2.0f;
        float forwardVelocity = 0.0f;
        float damageMultiplier = 0.0f;
    };

    struct VisualsConfig {
        bool showSpeedBar = true;
        bool showHints = true;
    };

    struct Config {
        VisualsConfig visuals;
        MainFlightStateConfig mainFlightState;
        TakeOffTransitionStateConfig takeOffTransitionState;
        ControlledFallStateConfig controlledFallState;
    };

    inline Config config{};

    inline Config& Get() { return config; }
}
