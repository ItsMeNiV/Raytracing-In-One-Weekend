#pragma once
#include <memory>
#include <vector>

#include "glm/glm.hpp"
#include "Ray.h"

struct HitRecord
{
    glm::vec3 p;
    glm::vec3 normal;
	float t;
    bool frontFace;

    inline void setFaceNormal(const Ray& r, const glm::vec3& outwardNormal)
    {
        frontFace = glm::dot(r.direction, outwardNormal) < 0;
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
    Sphere(glm::vec3 cen, double r) : center(cen), radius(r) {};

    virtual bool Hit(
        const Ray& r, float tMin, float tMax, HitRecord& rec) const override
    {
        glm::vec3 oc = r.origin - center;
        float a = glm::dot(r.direction, r.direction);
        float halfB = glm::dot(oc, r.direction);
        float c = glm::dot(oc, oc) - radius * radius;

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

        return true;
    }

public:
    glm::vec3 center;
    float radius;
};

class HittableList : public Hittable
{
public:
    HittableList() = default;
    HittableList(shared_ptr<Hittable> object) { add(object); }

    void clear() { objects.clear(); }
    void add(shared_ptr<Hittable> object) { objects.push_back(object); }

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

    std::vector<shared_ptr<Hittable>> objects;
};