#include "TakeOffTransitionState.h"

#include "Configuration.h"
#include "DrawDebug.h"
#include "FlightManager.h"
#include "Movement.h"
#include "PlayerBody.h"
#include "Raycast.h"
#include "ScreenLog.h"

RE::hkVector4 TakeOffTransitionState::Tick(float dt)
{
    const auto& config = Configuration::Get().takeOffTransitionState;

    if (accelerating) {
        takeOffBody.applyForce();
    }
    auto velocity = takeOffBody.update(dt);
    if (!accelerating && velocity <= config.transitionVelocityThreshold) {
        FlightManager::SetState(nextState);
        return {};
    }
    RE::hkVector4 velocityVector(0.0f, 0.0f, velocity, 0.0f);
    if (!accelerating) {
        velocityVector = velocityVector + nextState->Tick(dt);
    }
    PlayerBody::SetVelocity(velocityVector);
    auto player = RE::PlayerCharacter::GetSingleton();
    auto playerPosition = player->GetPosition();
    auto down = RE::NiPoint3(RE::NI_HALF_PI, 0, 0);
    auto hit = Raycast::CastRay(playerPosition, down, config.groundRaycastDistance);
    DrawDebug::DrawLine(playerPosition, hit.position);
    if (playerPosition.z - hit.position.z > config.minHeightAboveGround) {
        accelerating = false;
    }

    return velocityVector;
}

void TakeOffTransitionState::End(bool hasCollided) {}

bool TakeOffTransitionState::OnInput(RE::InputEvent* event)
{
    if (!accelerating) {
        return nextState->OnInput(event);
    }

    return false;
}

float TakeOffTransitionState::GetSpeed() { return nextState->GetSpeed(); }

float TakeOffTransitionState::GetDamageMultiplier() { return Configuration::Get().takeOffTransitionState.damageMultiplier; }

float TakeOffTransitionState::GetMaxSpeed() { return nextState->GetMaxSpeed(); }

void TakeOffTransitionState::MoveUp() {}

void TakeOffTransitionState::MoveDown() {}
