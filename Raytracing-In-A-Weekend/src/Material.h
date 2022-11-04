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

class Dielectric : public Material
{
public:
    Dielectric(float indexOfRefraction) : ir(indexOfRefraction) {}

    virtual bool scatter(const Ray& rIn, const HitRecord& rec, glm::vec3& attenuation, Ray& scattered) const override
    {
        attenuation = glm::vec3(1.0f, 1.0f, 1.0f);
        float refractionRatio = rec.frontFace ? (1.0f / ir) : ir;

        glm::vec3 unitDirection = unitVector(rIn.direction);
        float cosTheta = fmin(vecDot(-unitDirection, rec.normal), 1.0f);
        float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

        bool cannotRefract = refractionRatio * sinTheta > 1.0;
        glm::vec3 direction;

        if (cannotRefract || reflectance(cosTheta, refractionRatio) > randomFloat())
            direction = glm::reflect(unitDirection, rec.normal);
        else
            direction = refract(unitDirection, rec.normal, refractionRatio);

        scattered = Ray(rec.p, direction);
        return true;
    }

public:
    float ir; //Index of Refraction 

private:
    static float reflectance(float cosine, float refIdx)
    {
        float r0 = (1 - refIdx) / (1 + refIdx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};