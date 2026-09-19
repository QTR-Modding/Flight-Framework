#include "Raycast.h"
#include "RayCollector.h"

static RE::NiPoint3 angles2dir(const RE::NiPoint3& angles) {
    RE::NiPoint3 ans;

    const float sinx = sinf(angles.x);
    const float cosx = cosf(angles.x);
    const float sinz = sinf(angles.z);
    const float cosz = cosf(angles.z);

    ans.x = cosx * sinz;
    ans.y = cosx * cosz;
    ans.z = -sinx;

    return ans;
}


static RE::NiPoint3 rotate(const RE::NiPoint3& A, const RE::NiPoint3& angles) {
    RE::NiMatrix3 R;
    R.EulerAnglesToAxesZXY(angles);
    return R * A;
}
static RE::NiPoint3 rotate(const float r, const RE::NiPoint3& angles) { return angles2dir(angles) * r; }

Raycast::RayCastHit Raycast::CastRay(const RE::NiPoint3& start, const RE::NiPoint3& end) {
    auto havokWorldScale = RE::bhkWorld::GetWorldScale();
    RE::bhkPickData pick_data;

    const RE::NiPoint3 ray_start = start;
    RE::NiPoint3 ray_end = end;
    pick_data.rayInput.from = ray_start * havokWorldScale;
    pick_data.rayInput.to = ray_end * havokWorldScale;

    const auto dif = ray_start - ray_end;

    auto collector = RayCollector();
    collector.Reset();
    pick_data.closestRayHitCollector = reinterpret_cast<RE::hkpClosestRayHitCollector*>(&collector);

    const auto ply = RE::PlayerCharacter::GetSingleton();
    if (!ply->parentCell) return {{}, nullptr, false};

    if (auto physicsWorld = ply->parentCell->GetbhkWorld()) {
        physicsWorld->PickObject(pick_data);
    }

    RayCollector::HitResult best = {};
    best.hitFraction = 1.0f;
    RE::NiPoint3 bestPos = {};

    for (auto& hit : collector.GetHits()) {
        const auto pos = (dif * hit.hitFraction) + ray_start;
        if (best.body == nullptr) {
            best = hit;
            bestPos = pos;
            continue;
        }

        if (hit.hitFraction < best.hitFraction) {
            best = hit;
            bestPos = pos;
        }
    }

    if (!best.body) {
        return Raycast::RayCastHit(ray_end, nullptr, false);
    }

    auto hitpos = ray_start + (ray_end - ray_start) * best.hitFraction;

    if (const auto av = best.getAVObject()) {
        auto ref = av->GetUserData();

        return {hitpos, ref, true};
    }
    return Raycast::RayCastHit(hitpos, nullptr, true);
}
Raycast::RayCastHit Raycast::CastRay(const RE::NiPoint3& position, const RE::NiPoint3& angle, float length) {
    RE::NiPoint3 end = rotate(length, angle) + position;
    return CastRay(position, end);
}
