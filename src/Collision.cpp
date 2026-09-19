#include "Collision.h"
#include "DrawDebug.h"
#include "Raycast.h"
#include "ScreenLog.h"
#include "Utils.h"
#include "PlayerBody.h"
#include "PromptManager.h"

namespace {
    constexpr float baseCollisionDamage = 80.0f;
}

bool Collision::Apply(RE::hkVector4 velocity, float damageMultiplier, float damageThreshold) {
    auto player = RE::PlayerCharacter::GetSingleton();
    auto pos = player->GetPosition() + RE::NiPoint3{0, 0, 100};
    int numRays = 32;
    float radius = 150.0f;
    bool hasCollieded = false;

    RE::hkVector4 accumulatedNormal(0, 0, 0, 0);
    int hitCount = 0;

    for (int i = 0; i < numRays; ++i) {
        float u = float(i) / float(numRays);
        float theta = u * 2.0f * 3.14159265f;

        for (int j = 0; j < numRays; ++j) {
            float v = float(j) / float(numRays);
            float phi = v * 3.14159265f;

            float x = sin(phi) * cos(theta);
            float y = sin(phi) * sin(theta);
            float z = cos(phi);

            RE::hkVector4 dir(x, y, z, 0);
            float len = sqrtf(x * x + y * y + z * z);
            if (len == 0.0f) continue;
            dir = dir / len;

            auto start = pos;
            auto end = pos + RE::NiPoint3{x, y, z} * radius;

            auto hit = Raycast::CastRay(start, end);
            if (hit.hasHit) {
                hasCollieded = true;
                accumulatedNormal = accumulatedNormal - dir;
                hitCount++;
            }
        }
    }

    if (hitCount == 0) return hasCollieded;

    float nlen = sqrtf(accumulatedNormal.Dot3(accumulatedNormal));

    if (nlen == 0.0f) return hasCollieded;

    accumulatedNormal = accumulatedNormal / nlen;

    float normalSpeed = velocity.Dot3(accumulatedNormal);

    if (normalSpeed >= 0.0f) {
        return hasCollieded;
    }

    const float impactSpeed = -normalSpeed;
    if (impactSpeed <= damageThreshold) {
        return hasCollieded;
    }

    if (damageMultiplier > 0.0f) {
        const float impactScale = damageThreshold > 0.0f ? impactSpeed / damageThreshold : 1.0f;
        const float damage = baseCollisionDamage * impactScale * damageMultiplier;
        player->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kHealth, damage);
    }

    PromptManager::StopFlying();
    return hasCollieded;
}
