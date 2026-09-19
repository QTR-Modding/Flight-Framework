#include "FlightManager.h"
#include "FlightInput.h"

#include <shared_mutex>

#include "Collision.h"
#include "Configuration.h"
#include "DrawDebug.h"
#include "PlayerBody.h"
#include "PromptManager.h"
#include "Utils.h"
#include "PromptManager.h"
#include "InputConfig.h"
#include "SkyPromptClient.h"
FlyingState* currentState = nullptr;
std::shared_mutex mtx;
void FlightManager::Tick(float dt) {
    SkyPromptClient::Update();
    FlightInput::Update();
	if (currentState) {
        std::shared_lock<std::shared_mutex> lock(mtx);
        if (downPressed) {
            currentState->MoveDown();
        }
        if (upPressed) {
            currentState->MoveUp();
        }
        DrawDebug::Clean();
		auto velocity = currentState->Tick(dt);
        PlayerBody::SetVelocity(velocity);
        auto hasCollided = Collision::Apply(velocity, currentState->GetDamageMultiplier(), 25.0f);
        currentState->End(hasCollided);
	}
}

bool FlightManager::OnInput(RE::InputEvent* event) {

    if (auto button = event->AsButtonEvent()) {
        if (event->GetDevice() == RE::INPUT_DEVICE::kKeyboard) {
            if (button->IsDown() && button->GetIDCode() == InputConfig::Get("skyprompt.hideShowMenu", RE::INPUT_DEVICE::kKeyboard)) {
                //PromptManager::SetNonHidden();
            }
        }
        if (event->GetDevice() == RE::INPUT_DEVICE::kGamepad) {
            if (button->IsDown() && button->GetIDCode() == InputConfig::Get("skyprompt.hideShowMenu", RE::INPUT_DEVICE::kGamepad)) {
                //PromptManager::SetNonHidden();
            }
        }
    }

    if (currentState) {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return currentState->OnInput(event);
    }

	return false; 
}

void FlightManager::SetState(FlyingState* state) {
    SKSE::GetTaskInterface()->AddTask([state]() { 
        std::unique_lock<std::shared_mutex> lock(mtx);
        Utils::SetPlayerFlying(state != nullptr);
        currentState = state;
    });
}

void FlightManager::ExitFlight() {
    FlightManager::SetState(nullptr);
}

bool FlightManager::IsFlying() { return currentState == mainFlighState; }

FlyingState* FlightManager::GetCurrentState() { return currentState; }

void FlightManager::SetFreeFlight() {
    const auto& mainFlightConfig = Configuration::Get().mainFlightState;
    const auto& takeOffConfig = Configuration::Get().takeOffTransitionState;

    takeOffTransitionState->nextState = FlightManager::mainFlighState;
    takeOffTransitionState->takeOffBody.maxSpeed = takeOffConfig.bodyMaxSpeed;
    takeOffTransitionState->takeOffBody.timeToReachMaxSpeed = takeOffConfig.bodyTimeToReachMaxSpeed;
    takeOffTransitionState->takeOffBody.damping = takeOffConfig.bodyDamping;
    takeOffTransitionState->takeOffBody.resetForce();
    mainFlighState->forwardSpeed.resetForce();
    mainFlighState->leftRightSpeed.resetForce();
    mainFlighState->upAndDownSpeed.resetForce();
    mainFlighState->maxSpeed = mainFlightConfig.defaultMaxSpeed;
    mainFlighState->forwardSpeed.maxSpeed = mainFlightConfig.initialDirectionalMaxSpeed;
    mainFlighState->leftRightSpeed.maxSpeed = mainFlightConfig.initialDirectionalMaxSpeed;
    mainFlighState->upAndDownSpeed.maxSpeed = mainFlightConfig.initialDirectionalMaxSpeed;
    mainFlighState->lastState = "";
    takeOffTransitionState->accelerating = true;
    SetState(FlightManager::mainFlighState); // takeOffTransitionState
}

void FlightManager::SetControlledFall() {
    FlightManager::landTransitionState->speed = Configuration::Get().controlledFallState.initialSpeed;
    FlightManager::landTransitionState->upAndDownSpeed.resetForce();
    FlightManager::SetState(FlightManager::landTransitionState);
}

void FlightManager::DowmPressed() { downPressed = true; }

void FlightManager::DowmReleased() {
    downPressed = false;
}

void FlightManager::UpPressed() { upPressed = true; }

void FlightManager::UpReleased() { upPressed = false; }
