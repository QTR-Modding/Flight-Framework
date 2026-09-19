#pragma once


namespace PromptManager {
    inline bool isVisible = false;
    inline bool isFlying = false;
    inline bool canFly = false;

    void ExitFlight();
    void StopFlying();
    void StartFlying();
    void EnableFligt();
    void DisableFlight();

    void Update();
}
