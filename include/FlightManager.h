#pragma once
#include "FlyingState.h"

#include "LandTransitionState.h"
#include "TakeOffTransitionState.h"
#include "MainFlightState.h"
namespace FlightManager {
    inline bool downPressed = false;
    inline bool upPressed = false;
    inline MainFlightState* mainFlighState = new MainFlightState();
    inline LandTransitionState* landTransitionState = new LandTransitionState();
    inline TakeOffTransitionState* takeOffTransitionState = new TakeOffTransitionState();

    void Tick(float dt);
    bool OnInput(RE::InputEvent* event);
    void SetState(FlyingState* state);
    void ExitFlight();
    bool IsFlying();
    FlyingState* GetCurrentState();

    void SetFreeFlight();
    void SetControlledFall();


    void DowmPressed();
    void DowmReleased();
    void UpPressed();
    void UpReleased();

}
