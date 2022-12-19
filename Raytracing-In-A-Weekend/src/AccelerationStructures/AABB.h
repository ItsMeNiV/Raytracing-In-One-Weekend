#pragma once
#include "Core/RTWeekend.h"

class AABB
{
public:
    AABB() = default;
    AABB(const glm::vec3& a, const glm::vec3& b) : minimum(a), maximum(b) {}

    bool Hit(const Ray& r, float tMin, float tMax) const
    {
        for (int a = 0; a < 3; a++) {
            auto invD = 1.0f / r.direction[a];
            auto t0 = (minimum[a] - r.origin[a]) * invD;
            auto t1 = (maximum[a] - r.origin[a]) * invD;
            if (invD < 0.0f)
                std::swap(t0, t1);
            tMin = t0 > tMin ? t0 : tMin;
            tMax = t1 < tMax ? t1 : tMax;
            if (tMax <= tMin)
                return false;
        }
        return true;
    }

public:
    glm::vec3 minimum, maximum;
};

AABB surroundingBox(AABB box0, AABB box1);