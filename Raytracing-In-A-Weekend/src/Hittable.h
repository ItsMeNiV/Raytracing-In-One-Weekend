#pragma once
#include <memory>
#include <vector>

#include "RTWeekend.h"

class Material;

struct HitRecord
{
    glm::vec3 p;
    glm::vec3 normal;
    std::shared_ptr<Material> matPtr;
	float t;
    bool frontFace;
    float u;
    float v;

    inline void setFaceNormal(const Ray& r, const glm::vec3& outwardNormal)
    {
        frontFace = dot(r.direction, outwardNormal) < 0;
        normal = frontFace ? outwardNormal : -outwardNormal;
    }
};

class Hittable
{
public:
	virtual bool Hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const = 0;
};

class Triangle : public Hittable
{
public:
    Triangle() {}
    Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, std::shared_ptr<Material> material, std::string&& dbgName) : v0(v0), v1(v1), v2(v2), t0(-1, -1, -1), t1(-1, -1, -1), t2(-1, -1, -1), matPtr(material), debugName(std::move(dbgName)) {}
    Triangle(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2, glm::vec3 t0, glm::vec3 t1, glm::vec3 t2, std::shared_ptr<Material> material, std::string&& dbgName)
        : v0(v0), v1(v1), v2(v2), t0(t0), t1(t1), t2(t2), matPtr(material), debugName(std::move(dbgName)) {}

    virtual bool Hit(
        const Ray& r, float tMin, float tMax, HitRecord& rec) const override
    {
        const float EPSILON = 1e-8;
        glm::vec3 edge1, edge2, h, s, q;
        float a, f, u, v;
        edge1 = v1 - v0;
        edge2 = v2 - v0;
        h = cross(r.direction, edge2);
        a = dot(edge1, h);
        if (a > -EPSILON && a < EPSILON)
            return false;    // This ray is parallel to this triangle.
        f = 1.0f / a;
        s = r.origin - v0;
        u = f * dot(s, h);
        if (u < 0.0f || u > 1.0f)
            return false;
        q = cross(s, edge1);
        v = f * dot(r.direction, q);
        if (v < 0.0f || u + v > 1.0f)
            return false;
        // At this stage we can compute t to find out where the intersection point is on the line.
        float t = f * dot(edge2, q);
        if (t < tMin || tMax < t)
            return false; //Intersection is not closer than the closest so far

        if (t > EPSILON) // ray intersection
        {
            rec.t = t;
            rec.p = r.At(rec.t);
            rec.setFaceNormal(r, glm::normalize(cross(edge2, edge1)));
            if(t0.x != -1.0f)
            {
                glm::vec3 barycentricCoord = u * t0 + v * t1 + (1 - u - v) * t2;
                rec.u = barycentricCoord.x;
                rec.v = barycentricCoord.y;
            }
            else
            {
                rec.u = u;
                rec.v = v;
            }
            rec.matPtr = matPtr;
            return true;
        }
        else // This means that there is a line intersection but not a ray intersection.
            return false;
    }

public:
    glm::vec3 v0, v1, v2; //Vertex positions
    glm::vec3 t0, t1, t2; //Texture coordinates
    std::shared_ptr<Material> matPtr;
    std::string debugName;
};

class Sphere : public Hittable
{
public:
    Sphere() = default;
    Sphere(glm::vec3 cen, float r, std::shared_ptr<Material> material) : center(cen), radius(r), matPtr(material) {};

    virtual bool Hit(
        const Ray& r, float tMin, float tMax, HitRecord& rec) const override
    {
        glm::vec3 oc = r.origin - center;
        float a = glm::dot(r.direction, r.direction);
        float halfB = dot(oc, r.direction);
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