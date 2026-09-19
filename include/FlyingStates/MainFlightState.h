#pragma once
#include "FlyingState.h"
#include "Body.h"
#include "Configuration.h"

class MainFlightState : public FlyingState {
public:
    bool IsAttacking = false;
    float maxSpeed = Configuration::Get().mainFlightState.defaultMaxSpeed;
    std::string lastState = "";
    Body forwardSpeed;
    Body leftRightSpeed;
    Body upAndDownSpeed;
    RE::hkVector4 Tick(float dt) override;
    void End(bool hasCollided) override;
    bool OnInput(RE::InputEvent* event) override;
    float GetDamageMultiplier() override;
    float GetSpeed() override;
    float GetMaxSpeed() override;
    void MoveUp() override;
    void MoveDown() override;
};
