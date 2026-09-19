#pragma once

namespace FlightInput {
    void Install();
    bool OnInput(RE::InputEvent* event);
    void Update();
    // Cancel actions but retain consumed buttons until their physical release.
    void Cancel();
    // Forget device state when starting/loading a game or disconnecting.
    void Reset();
}
