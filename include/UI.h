#pragma once
#include "SKSEMenuFramework.h"
#include "Configuration.h"
namespace UI {
    void Register();
    namespace Configuration {
        void __stdcall Render();
    }
    namespace Visuals {
        void __stdcall Render();
    }
    namespace KeyBindings {
        void __stdcall Render();
    }
    namespace Main {
        inline RE::INPUT_DEVICE lastDevice = RE::INPUT_DEVICE::kKeyboard;
        void __stdcall Hud();
        void __stdcall Render();
        bool _stdcall OnInput(RE::InputEvent* event);
    }
};
