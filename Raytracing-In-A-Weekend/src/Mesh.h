#pragma once
#include <vector>
#include "Hittable.h"

class Mesh : public Hittable
{
public:
	Mesh(Vec3 pos, const char* location);

    virtual bool Hit(const Ray& r, double tMin, double tMax, HitRecord& rec) const override
    {
        HitRecord tempRec;
        bool hitAnything = false;
        auto closestSoFar = tMax;

        for (const auto& tri : triangles) {
            if (tri.Hit(r, tMin, closestSoFar, tempRec)) {
                hitAnything = true;
                closestSoFar = tempRec.t;
                rec = tempRec;
            }
        }

        return hitAnything;
    }

private:
	Vec3 position;
	std::vector<Triangle> triangles;

};