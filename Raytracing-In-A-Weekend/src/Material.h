#pragma once
#include "RTWeekend.h"

struct HitRecord;

class Material
{
public:
    virtual bool scatter(const Ray& rIn, const HitRecord& rec, glm::vec3& attenuation, Ray& scattered) const = 0;
};

class Lambertian : public Material
{
public:
    Lambertian(const glm::vec3& a) : albedo(a) {}

    virtual bool scatter(const Ray& rIn, const HitRecord& rec, glm::vec3& attenuation, Ray& scattered) const override
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
    glm::vec3 albedo;
};

class Metal : public Material
{
public:
    Metal(const glm::vec3& a, float f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(const Ray& rIn, const HitRecord& rec, glm::vec3& attenuation, Ray& scattered) const override
    {
        glm::vec3 reflected = glm::reflect(unitVector(rIn.direction), rec.normal);
        scattered = Ray(rec.p, reflected + fuzz*randomVecInUnitSphere());
        attenuation = albedo;
        return (vecDot(scattered.direction, rec.normal) > 0);
    }

private:
    glm::vec3 albedo;
    float fuzz;
};