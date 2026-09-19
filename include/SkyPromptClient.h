#pragma once


namespace SkyPromptClient {
    void Install();
    void PrepareForLoad();
    void Update();
    void SetInputDevice(RE::INPUT_DEVICE device);
    void ShowFly();
    void ShowExit();
    void Hide();
    void RefreshBindings();
    void RefreshMenuVisibility();
}
