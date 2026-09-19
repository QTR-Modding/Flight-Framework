#pragma once

class FlyingState {
public:
    virtual RE::hkVector4 Tick(float dt) = 0;
    virtual void End(bool hasCollided) = 0;
    virtual bool OnInput(RE::InputEvent* event) = 0;
    virtual float GetSpeed() = 0;
    virtual float GetMaxSpeed() = 0;
    virtual float GetDamageMultiplier() = 0;
    virtual void MoveUp() = 0;
    virtual void MoveDown() = 0;
};