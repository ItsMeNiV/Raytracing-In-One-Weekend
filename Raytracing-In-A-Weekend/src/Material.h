#pragma once
#include "RTWeekend.h"
#include "Texture.h"

struct HitRecord;

class Material
{
public:
    virtual bool scatter(const Ray& rIn, const HitRecord& rec, glm::vec3& attenuation, Ray& scattered) const = 0;

    virtual glm::vec3 emitted(float u, float v, const glm::vec3& p) const {
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }
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
        glm::vec3 reflected = reflect(glm::normalize(rIn.direction), rec.normal);
        scattered = Ray(rec.p, reflected + fuzz*randomVecInUnitSphere());
        attenuation = albedo;
        return (dot(scattered.direction, rec.normal) > 0);
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

        glm::vec3 unitDirection = glm::normalize(rIn.direction);
        float cosTheta = fmin(dot(-unitDirection, rec.normal), 1.0f);
        float sinTheta = sqrt(1.0f - cosTheta * cosTheta);

        bool cannotRefract = refractionRatio * sinTheta > 1.0f;
        glm::vec3 direction;

        if (cannotRefract || reflectance(cosTheta, refractionRatio) > randomFloat())
            direction = reflect(unitDirection, rec.normal);
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

class DiffuseLight : public Material
{
public:
    DiffuseLight(glm::vec3 c) : emit(c) {}

    virtual bool scatter(const Ray& rIn, const HitRecord& rec, glm::vec3& attenuation, Ray& scattered) const override
    {
        return false;
    }

    virtual glm::vec3 emitted(float u, float v, const glm::vec3& p) const override {
        return emit;
    }

public:
    glm::vec3 emit;
};

class PBRMaterial : public Material
{
public:
    PBRMaterial(std::shared_ptr<Texture> diffuse) : diffuseTexture(diffuse) {}

    virtual bool scatter(const Ray& rIn, const HitRecord& rec, glm::vec3& attenuation, Ray& scattered) const override
    {
        //Override normal with normalmap if available
        glm::vec3 normal(0.0f, 0.0f, 0.0f);
        if (normalTexture)
        {
            glm::mat3 normalMatrix(glm::transpose(glm::inverse(rec.modelMatrix)));
            glm::vec3 T = glm::normalize(normalMatrix * rec.tangent);
            glm::vec3 B = glm::normalize(normalMatrix * rec.bitangent);
            glm::vec3 N = glm::normalize(rec.normal);
            glm::mat3 TBN(T, B, N);

            normal = normalTexture->At(rec.u, rec.v) * 2.0f - 1.0f;
            normal = glm::normalize(TBN * normal);
        }
        else
        {
            normal = rec.normal;
        }
        normal = rec.normal; //Normalmapping doesn't work properly yet

        attenuation = diffuseTexture->At(rec.u, rec.v);

        if (roughnessTexture)
        {
            glm::vec3 reflected = reflect(glm::normalize(rIn.direction), normal);
            scattered = Ray(rec.p, reflected + roughnessTexture->At(rec.u, rec.v) * randomVecInUnitSphere());
            return (dot(scattered.direction, normal) > 0);
        }
        else
        {
            auto scatterDirection = normal + randomUnitVector();

            if (vecNearZero(scatterDirection))
                scatterDirection = normal;

            scattered = Ray(rec.p, scatterDirection);
            return true;
        }
    }

    void setRoughnessTexture(std::shared_ptr<Texture> rough) { roughnessTexture = rough; }
    void setNormalTexture(std::shared_ptr<Texture> normal) {normalTexture = normal; }

private:
    std::shared_ptr<Texture> diffuseTexture;
    std::shared_ptr<Texture> roughnessTexture;
    std::shared_ptr<Texture> normalTexture;

};