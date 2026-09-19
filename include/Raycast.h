#pragma once

namespace Raycast {

    struct RayCastHit {
        RE::NiPoint3 position;
        RE::TESObjectREFR* reference;
        bool hasHit;
    };

    RayCastHit CastRay(const RE::NiPoint3& start, const RE::NiPoint3& end);
    RayCastHit CastRay(const RE::NiPoint3& position, const RE::NiPoint3& direction, float length);
}


