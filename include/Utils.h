#pragma once

namespace Utils {
    RE::NiPoint3 GetCameraAngle();
    bool IsThirdPerson();
    bool SetPlayerFlying(bool value);
    bool SetFlying(RE::Actor* a_actor, bool value);
    bool IsMenuOpen();
}