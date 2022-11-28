#pragma once
#include <memory>
#include <vector>

#include "RTWeekend.h"

class Material;

struct HitRecord
{
    Vec3 p;
    Vec3 normal;
    std::shared_ptr<Material> matPtr;
	double t;
    bool frontFace;
    double u;
    double v;

    inline void setFaceNormal(const Ray& r, const Vec3& outwardNormal)
    {
        frontFace = dot(r.direction, outwardNormal) < 0;
        normal = frontFace ? outwardNormal : -outwardNormal;
    }
};

class Hittable
{
public:
	virtual bool Hit(const Ray& r, double tMin, double tMax, HitRecord& rec) const = 0;
};

class Triangle : public Hittable
{
public:
    Triangle() {}
    Triangle(Vec3 v0, Vec3 v1, Vec3 v2, std::shared_ptr<Material> material, std::string&& dbgName) : v0(v0), v1(v1), v2(v2), matPtr(material), debugName(std::move(dbgName)) {}

    virtual bool Hit(
        const Ray& r, double tMin, double tMax, HitRecord& rec) const override
    {
        const double EPSILON = 1e-8;
        Vec3 edge1, edge2, h, s, q;
        double a, f, u, v;
        edge1 = v1 - v0;
        edge2 = v2 - v0;
        h = cross(r.direction, edge2);
        a = dot(edge1, h);
        if (a > -EPSILON && a < EPSILON)
            return false;    // This ray is parallel to this triangle.
        f = 1.0 / a;
        s = r.origin - v0;
        u = f * dot(s, h);
        if (u < 0.0 || u > 1.0)
            return false;
        q = cross(s, edge1);
        v = f * dot(r.direction, q);
        if (v < 0.0 || u + v > 1.0)
            return false;
        // At this stage we can compute t to find out where the intersection point is on the line.
        double t = f * dot(edge2, q);
        if (t > EPSILON) // ray intersection
        {
            rec.t = t;
            rec.p = r.At(rec.t);
            rec.setFaceNormal(r, unitVector(cross(edge2, edge1)));
            rec.matPtr = matPtr;
            return true;
        }
        else // This means that there is a line intersection but not a ray intersection.
            return false;
    }

public:
    Vec3 v0, v1, v2;
    std::shared_ptr<Material> matPtr;
    std::string debugName;
};

class Sphere : public Hittable
{
public:
    Sphere() {}
    Sphere(Vec3 cen, double r, std::shared_ptr<Material> material) : center(cen), radius(r), matPtr(material) {};

    virtual bool Hit(
        const Ray& r, double tMin, double tMax, HitRecord& rec) const override
    {
        Vec3 oc = r.origin - center;
        double a = r.direction.length_squared();
        double halfB = dot(oc, r.direction);
        double c = oc.length_squared() - radius * radius;

        double discriminant = halfB * halfB - a * c;
        if (discriminant < 0) return false;
        double sqrtd = sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        double root = (-halfB - sqrtd) / a;
        if (root < tMin || tMax < root) {
            root = (-halfB + sqrtd) / a;
            if (root < tMin || tMax < root)
                return false;
        }

        rec.t = root;
        rec.p = r.At(rec.t);
        Vec3 outwardNormal = (rec.p - center) / radius;
        rec.setFaceNormal(r, outwardNormal);
        rec.matPtr = matPtr;

        return true;
    }

public:
    Vec3 center;
    double radius;
    std::shared_ptr<Material> matPtr;
};

class HittableList : public Hittable
{
public:
    HittableList() = default;
    HittableList(std::shared_ptr<Hittable> object) { add(object); }

    void clear() { objects.clear(); }
    void add(std::shared_ptr<Hittable> object) { objects.push_back(object); }

    virtual bool Hit(const Ray& r, double tMin, double tMax, HitRecord& rec) const override
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