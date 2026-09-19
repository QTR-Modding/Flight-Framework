#include "LandTransitionState.h"

#include "FlightManager.h"
#include "Movement.h"
#include "Utils.h"

RE::hkVector4 LandTransitionState::Tick(float dt)
{
    const auto& config = Configuration::Get().controlledFallState;

    upAndDownSpeed.damping = config.inputDamping;
    upAndDownSpeed.maxSpeed = config.maxSpeed;
    upAndDownSpeed.timeToReachMaxSpeed = config.timeToReachMaxSpeed;
    upAndDownSpeed.applyForce();
    speed = upAndDownSpeed.update(dt);

    auto angle = Utils::GetCameraAngle();
    RE::hkVector4 velocity = Movement::GetVelocityTowardsDirection(angle, config.forwardVelocity);
    velocity.quad = _mm_setr_ps(velocity.quad.m128_f32[0], velocity.quad.m128_f32[1], speed, velocity.quad.m128_f32[3]);

    return velocity;
}

void LandTransitionState::End(bool hasCollided)
{
    if (hasCollided) {
        FlightManager::ExitFlight();
    }
}

bool LandTransitionState::OnInput(RE::InputEvent* event) { return false; }

float LandTransitionState::GetDamageMultiplier() { return Configuration::Get().controlledFallState.damageMultiplier; }

float LandTransitionState::GetSpeed() { return speed; }

float LandTransitionState::GetMaxSpeed() { return upAndDownSpeed.maxSpeed; }

void LandTransitionState::MoveUp() {}

void LandTransitionState::MoveDown() {}
