#pragma once

#include "FlyingState.h"
#include "Body.h"

class TakeOffTransitionState : public FlyingState {
public:
    bool accelerating = false;
    Body takeOffBody;
    FlyingState* nextState = nullptr;
    RE::hkVector4 Tick(float dt) override;
    void End(bool hasCollided) override;
    bool OnInput(RE::InputEvent* event) override;
    float GetSpeed() override;
    float GetDamageMultiplier() override;
    float GetMaxSpeed() override;
    void MoveUp() override;
    void MoveDown() override;
};