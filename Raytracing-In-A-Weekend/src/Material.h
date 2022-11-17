#pragma once
#include "RTWeekend.h"

struct HitRecord;

class Material
{
public:
    virtual bool scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const = 0;

    virtual Vec3 emitted(double u, double v, const Vec3& p) const {
        return Vec3(0, 0, 0);
    }
};

class Lambertian : public Material
{
public:
    Lambertian(const Vec3& a) : albedo(a) {}

    virtual bool scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const override
    {
        #ifdef HEMISPHERE_DIFFUSE
        auto scatterDirection = randomInHemisphere(rec.normal);
        #else
        auto scatterDirection = rec.normal + randomUnitVector();
        #endif

        if (vecNearZero(scatterDirection))
            scatterDirection = rec.normal;

        scattered = Ray(rec.p, scatterDirection);
        attenuation = albedo;
        return true;
    }

private:
    Vec3 albedo;
};

class Metal : public Material
{
public:
    Metal(const Vec3& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const override
    {
        Vec3 reflected = reflect(unitVector(rIn.direction), rec.normal);
        scattered = Ray(rec.p, reflected + fuzz*randomVecInUnitSphere());
        attenuation = albedo;
        return (dot(scattered.direction, rec.normal) > 0);
    }

private:
    Vec3 albedo;
    double fuzz;
};

class Dielectric : public Material
{
public:
    Dielectric(double indexOfRefraction) : ir(indexOfRefraction) {}

    virtual bool scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const override
    {
        attenuation = Vec3(1.0, 1.0, 1.0);
        double refractionRatio = rec.frontFace ? (1.0 / ir) : ir;

        Vec3 unitDirection = unitVector(rIn.direction);
        double cosTheta = fmin(dot(-unitDirection, rec.normal), 1.0);
        double sinTheta = sqrt(1.0 - cosTheta * cosTheta);

        bool cannotRefract = refractionRatio * sinTheta > 1.0;
        Vec3 direction;

        if (cannotRefract || reflectance(cosTheta, refractionRatio) > randomdouble())
            direction = reflect(unitDirection, rec.normal);
        else
            direction = refract(unitDirection, rec.normal, refractionRatio);

        scattered = Ray(rec.p, direction);
        return true;
    }

public:
    double ir; //Index of Refraction 

private:
    static double reflectance(double cosine, double refIdx)
    {
        double r0 = (1 - refIdx) / (1 + refIdx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};

class DiffuseLight : public Material
{
public:
    DiffuseLight(Vec3 c) : emit(c) {}

    virtual bool scatter(const Ray& rIn, const HitRecord& rec, Vec3& attenuation, Ray& scattered) const override
    {
        return false;
    }

    virtual Vec3 emitted(double u, double v, const Vec3& p) const override {
        return emit;
    }

public:
    Vec3 emit;
};