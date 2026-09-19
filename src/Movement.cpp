#include <glm/ext.hpp>
#include "Movement.h"

RE::hkVector4 Movement::GetVelocityTowardsDirection(RE::NiPoint3 direction, float speed) {
    float deltaX = speed * std::cos(-direction.z + glm::half_pi<float>());
    float deltaY = speed * std::sin(-direction.z + glm::half_pi<float>());
    float deltaZ = speed * std::sin(-direction.x);
    RE::hkVector4 velocity(deltaX, deltaY, deltaZ, 0);
    return velocity;
}

RE::hkVector4 Movement::GetVelocityTowardsDirection2d(RE::NiPoint3 direction, float speed, float strafe, float vertical) {
    float yaw = -direction.z;

    float forwardX = std::cos(yaw + glm::half_pi<float>());
    float forwardY = std::sin(yaw + glm::half_pi<float>());

    float rightX = std::cos(yaw);
    float rightY = std::sin(yaw);

    float deltaX = forwardX * speed + rightX * strafe;
    float deltaY = forwardY * speed + rightY * strafe;
    float deltaZ = vertical;

    return RE::hkVector4(deltaX, deltaY, deltaZ, 0.0f);
}

