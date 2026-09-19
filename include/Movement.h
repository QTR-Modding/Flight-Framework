#pragma once

namespace Movement {
    RE::hkVector4 GetVelocityTowardsDirection(RE::NiPoint3 direction, float speed);
    RE::hkVector4 GetVelocityTowardsDirection2d(RE::NiPoint3 direction, float speed, float strafe, float vertical);
}