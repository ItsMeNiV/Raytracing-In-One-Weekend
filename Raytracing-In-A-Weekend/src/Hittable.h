#pragma once
#include <memory>
#include <vector>

#include "glm/glm.hpp"
#include "Ray.h"
#include "RTWeekend.h"

class Material;

struct HitRecord
{
    glm::vec3 p;
    glm::vec3 normal;
    std::shared_ptr<Material> matPtr;
	float t;
    bool frontFace;

    inline void setFaceNormal(const Ray& r, const glm::vec3& outwardNormal)
    {
        frontFace = vecDot(r.direction, outwardNormal) < 0;
        normal = frontFace ? outwardNormal : -outwardNormal;
    }
};

class Hittable
{
public:
	virtual bool Hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const = 0;
};

class Sphere : public Hittable {
public:
    Sphere() {}
    Sphere(glm::vec3 cen, double r, std::shared_ptr<Material> material) : center(cen), radius(r), matPtr(material) {};

    virtual bool Hit(
        const Ray& r, float tMin, float tMax, HitRecord& rec) const override
    {
        glm::vec3 oc = r.origin - center;
        float a = vecLengthSquared(r.direction);
        float halfB = vecDot(oc, r.direction);
        float c = vecLengthSquared(oc) - radius * radius;

        float discriminant = halfB * halfB - a * c;
        if (discriminant < 0) return false;
        float sqrtd = sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        float root = (-halfB - sqrtd) / a;
        if (root < tMin || tMax < root) {
            root = (-halfB + sqrtd) / a;
            if (root < tMin || tMax < root)
                return false;
        }

        rec.t = root;
        rec.p = r.At(rec.t);
        glm::vec3 outwardNormal = (rec.p - center) / radius;
        rec.setFaceNormal(r, outwardNormal);
        rec.matPtr = matPtr;

        return true;
    }

public:
    glm::vec3 center;
    float radius;
    std::shared_ptr<Material> matPtr;
};

class HittableList : public Hittable
{
public:
    HittableList() = default;
    HittableList(std::shared_ptr<Hittable> object) { add(object); }

    void clear() { objects.clear(); }
    void add(std::shared_ptr<Hittable> object) { objects.push_back(object); }

    virtual bool Hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const override
    {
        HitRecord tempRec;
        bool hitAnything = false;
        auto closestSoFar = tMax;

        for (const auto& object : objects) {
            if (object->Hit(r, tMin, closestSoFar, tempRec)) {
                hitAnything = true;
                closestSoFar = tempRec.t;
                rec = tempRec;
            }
        }

        return hitAnything;
    }

    std::vector<std::shared_ptr<Hittable>> objects;
};