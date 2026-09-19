#include "PlayerBody.h"
#include "Utils.h"


void PlayerBody::SetVelocity(RE::hkVector4 velocity) {
    auto player = RE::PlayerCharacter::GetSingleton();

    auto playerController = player->GetCharController();
    playerController->context.currentState = RE::hkpCharacterStateTypes::kInAir;
    playerController->fallTime = 0;
    playerController->fallStartHeight = 0;
    playerController->SetLinearVelocityImpl(velocity);
}
