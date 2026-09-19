#pragma once
#include "FlyingState.h"
#include "Body.h"
#include "Configuration.h"

class LandTransitionState : public FlyingState {
public:
    Body upAndDownSpeed;
    float speed = Configuration::Get().controlledFallState.initialSpeed;
    RE::hkVector4 Tick(float dt) override;
    void End(bool hasCollided) override;
    bool OnInput(RE::InputEvent* event) override;
    float GetDamageMultiplier() override;
    float GetSpeed() override;
    float GetMaxSpeed() override;
    void MoveUp() override;
    void MoveDown() override;
};