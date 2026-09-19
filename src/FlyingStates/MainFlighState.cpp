#include "MainFlightState.h"

#include "Movement.h"
#include "Utils.h"

namespace {
    void ApplyDirectionalForce(Body& body, float targetSpeed, const Configuration::MainFlightStateConfig& config)
    {
        body.damping = config.inputDamping;
        body.maxSpeed = targetSpeed;
        body.timeToReachMaxSpeed = config.timeToReachMaxSpeed;
        body.applyForce();
    }
}

RE::hkVector4 MainFlightState::Tick(float dt)
{
    auto angle = Utils::GetCameraAngle();
    float forward = forwardSpeed.update(dt);
    float leftRight = leftRightSpeed.update(dt);
    float vertical = upAndDownSpeed.update(dt);

    RE::NiPoint3 velocityVector{ forward, leftRight, vertical };

    float len = velocityVector.Length();
    if (len > maxSpeed) {
        velocityVector *= maxSpeed / len;
    }

    auto velocity = Movement::GetVelocityTowardsDirection2d(angle, velocityVector.x, velocityVector.y, velocityVector.z);
    return velocity;
}

void MainFlightState::End(bool hasCollided)
{
}

bool MainFlightState::OnInput(RE::InputEvent* event)
{
    if (Utils::IsMenuOpen()) {
        return false;
    }

    bool block = false;
    if (event->device == RE::INPUT_DEVICE::kGamepad) {
        if (auto thumb = event->AsThumbstickEvent()) {
            const auto& config = Configuration::Get().mainFlightState;
            const bool isSprinting = RE::PlayerCharacter::GetSingleton()->AsActorState()->IsSprinting();
            if (isSprinting) {
                maxSpeed = config.sprintMaxSpeed;
            } else {
                maxSpeed = config.regularMaxSpeed;
            }
            float xSign = thumb->xValue < 0 ? -1.0f : 1.0f;
            float ySign = thumb->yValue < 0 ? -1.0f : 1.0f;
            ApplyDirectionalForce(leftRightSpeed, maxSpeed * xSign, config);
            ApplyDirectionalForce(forwardSpeed, maxSpeed * ySign, config);
        }
    }
    if (event->device == RE::INPUT_DEVICE::kKeyboard) {
        if (auto button = event->AsButtonEvent()) {
            const auto& config = Configuration::Get().mainFlightState;
            const bool isSprinting = RE::PlayerCharacter::GetSingleton()->AsActorState()->IsSprinting();
            if (isSprinting) {
                maxSpeed = config.sprintMaxSpeed;
            } else {
                maxSpeed = config.regularMaxSpeed;
            }

            if (button->GetIDCode() == RE::BSWin32KeyboardDevice::Key::kW) {
                ApplyDirectionalForce(forwardSpeed, maxSpeed, config);
            }
            if (button->GetIDCode() == RE::BSWin32KeyboardDevice::Key::kS) {
                ApplyDirectionalForce(forwardSpeed, -maxSpeed, config);
            }

            if (button->GetIDCode() == RE::BSWin32KeyboardDevice::Key::kD) {
                ApplyDirectionalForce(leftRightSpeed, maxSpeed, config);
            }

            if (button->GetIDCode() == RE::BSWin32KeyboardDevice::Key::kA) {
                ApplyDirectionalForce(leftRightSpeed, -maxSpeed, config);
            }
        }
    }
    return block;
}

float MainFlightState::GetDamageMultiplier() { return Configuration::Get().mainFlightState.damageMultiplier; }

float MainFlightState::GetSpeed()
{
    RE::NiPoint3 v{ forwardSpeed.speed, leftRightSpeed.speed, upAndDownSpeed.speed };
    return v.Length();
}

float MainFlightState::GetMaxSpeed() { return Configuration::Get().mainFlightState.defaultMaxSpeed; }

void MainFlightState::MoveUp() {
    const auto& config = Configuration::Get().mainFlightState;
    const bool isSprinting = RE::PlayerCharacter::GetSingleton()->AsActorState()->IsSprinting();
    if (isSprinting) {
        maxSpeed = config.sprintMaxSpeed;
    } else {
        maxSpeed = config.regularMaxSpeed;
    }
    ApplyDirectionalForce(upAndDownSpeed, maxSpeed, config);
}

void MainFlightState::MoveDown() {
    const auto& config = Configuration::Get().mainFlightState;
    const bool isSprinting = RE::PlayerCharacter::GetSingleton()->AsActorState()->IsSprinting();
    if (isSprinting) {
        maxSpeed = config.sprintMaxSpeed;
    } else {
        maxSpeed = config.regularMaxSpeed;
    }
    ApplyDirectionalForce(upAndDownSpeed, -maxSpeed, config);

}
