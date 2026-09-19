#include "Utils.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
RE::NiPoint3 QuaternionToEuler(const RE::NiQuaternion& q) {
    RE::NiPoint3 euler;

    const double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
    const double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
    euler.x = std::atan2(sinr_cosp, cosr_cosp);

    // Pitch (y-axis rotation)
    if (const double sinp = 2 * (q.w * q.y - q.z * q.x); std::abs(sinp) >= 1)
        euler.y = std::copysign(glm::pi<float>() / 2, sinp);
    else
        euler.y = std::asin(sinp);

    // Yaw (z-axis rotation)
    const double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
    const double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
    euler.z = std::atan2(siny_cosp, cosy_cosp);

    euler.x = euler.x * -1;
    // euler.y = euler.y;
    euler.z = euler.z * -1;

    return euler;
}

RE::NiPoint3 Utils::GetCameraAngle() {
    const RE::PlayerCamera* camera = RE::PlayerCamera::GetSingleton();
    const auto thirdPerson = reinterpret_cast<RE::ThirdPersonState*>(camera->GetRuntimeData().cameraStates[RE::CameraState::kThirdPerson].get());
    const auto firstPerson = reinterpret_cast<RE::FirstPersonState*>(camera->GetRuntimeData().cameraStates[RE::CameraState::kFirstPerson].get());

    RE::NiQuaternion rotation;
    RE::NiPoint3 translation;
    if (camera->currentState->id == RE::CameraState::kFirstPerson) {
        firstPerson->GetRotation(rotation);
        translation += firstPerson->dampeningOffset;
    } else if (camera->currentState->id == RE::CameraState::kThirdPerson) {
        rotation = thirdPerson->rotation;
    } else {
        return {};
    }
    return QuaternionToEuler(rotation);
}

bool Utils::IsThirdPerson() { 
    const RE::PlayerCamera* camera = RE::PlayerCamera::GetSingleton();
    const auto thirdPerson = reinterpret_cast<RE::ThirdPersonState*>(camera->GetRuntimeData().cameraStates[RE::CameraState::kThirdPerson].get());
    const auto firstPerson = reinterpret_cast<RE::FirstPersonState*>(camera->GetRuntimeData().cameraStates[RE::CameraState::kFirstPerson].get());
    return camera->currentState->id == RE::CameraState::kThirdPerson;
}

bool Utils::SetPlayerFlying(bool value) { 
    return SetFlying(RE::PlayerCharacter::GetSingleton(), value); }

bool Utils::SetFlying(RE::Actor* a_actor, bool value) {
    if (const auto animGraphHolder = static_cast<RE::IAnimationGraphManagerHolder*>(a_actor)) {
        if (animGraphHolder->SetGraphVariableBool("FlightFramework_IsFlying", value)) {
            return true;
        }
        return false;
    }
    return false;
}

bool Utils::IsMenuOpen() {
    const auto ui = RE::UI::GetSingleton();
    if (ui->IsApplicationMenuOpen() || ui->IsItemMenuOpen() || ui->IsModalMenuOpen() || ui->GameIsPaused()) {
        return true;
    }
    return false;
}
