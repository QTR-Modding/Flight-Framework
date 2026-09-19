#include "PromptManager.h"
#include "SkyPromptClient.h"
#include "FlightManager.h"
#include "FlightInput.h"

namespace {
    void UpdatePrompts()
    {
        if (!PromptManager::isVisible) {
            SkyPromptClient::Hide();
            return;
        }

        if (PromptManager::isFlying) {
            SkyPromptClient::ShowExit();
        } else {
            SkyPromptClient::ShowFly();
        }
    }
}

void PromptManager::ExitFlight() {
    FlightInput::Cancel();
    isFlying = false;
    Update();
}

void PromptManager::StopFlying()
{
    FlightInput::Cancel();
    isFlying = false;
    UpdatePrompts();
    FlightManager::ExitFlight();
}

void PromptManager::StartFlying() {
    isFlying = true;
    Update();
}

void PromptManager::EnableFligt()
{
    if (canFly && isVisible) {
        return;
    }

    canFly = true;
    isVisible = true;
    Update();
}

void PromptManager::DisableFlight() { 
    if (!canFly && !isVisible && !isFlying) {
        return;
    }

    FlightInput::Cancel();
    canFly = false;
    isVisible = false;
    if (isFlying) {
        ExitFlight();
    } else {
        Update();
    }
}

void PromptManager::Update() {
    UpdatePrompts();

    if (isFlying) {
        if (!FlightManager::IsFlying()) {
            FlightManager::SetFreeFlight();
        }
    } else {
        if (FlightManager::IsFlying()) {
            FlightManager::SetControlledFall();
        }
    }
}
