#pragma once
#include <memory>
#include <vector>

#include "RTWeekend.h"
#include "AABB.h"

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
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::mat4 modelMatrix;

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
    virtual bool BoundingBox(AABB& outputBox) const = 0;
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 textureCoord;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

class Triangle : public Hittable
{
public:
    Triangle() {}
    Triangle(Vertex vert0, Vertex vert1, Vertex vert2, glm::mat4 modelMatrix, std::shared_ptr<Material> material, std::string&& dbgName)
        : modelMatrix(modelMatrix), matPtr(material), debugName(std::move(dbgName))
    {
        vertices[0] = vert0;
        vertices[1] = vert1;
        vertices[2] = vert2;
    }

    virtual bool Hit(
        const Ray& r, float tMin, float tMax, HitRecord& rec) const override
    {
        const float EPSILON = 1e-8;
        glm::vec3 edge1, edge2, h, s, q;
        float a, f, u, v;
        edge1 = vertices[1].position - vertices[0].position;
        edge2 = vertices[2].position - vertices[0].position;
        h = cross(r.direction, edge2);
        a = dot(edge1, h);
        if (a > -EPSILON && a < EPSILON)
            return false;    // This ray is parallel to this triangle.
        f = 1.0f / a;
        s = r.origin - vertices[0].position;
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

            if (vertices[0].normal.x == 0.0f && vertices[0].normal.y == 0.0f && vertices[0].normal.z == 0.0f)
            {
                rec.setFaceNormal(r, glm::normalize(cross(edge2, edge1)));
            }
            else
            {
                rec.normal = glm::normalize(u * vertices[0].normal + v * vertices[1].normal + (1 - u - v) * vertices[2].normal);
            }
            if(vertices[0].textureCoord.x != -1.0f)
            {
                glm::vec3 barycentricCoord = u * vertices[0].textureCoord + v * vertices[1].textureCoord + (1 - u - v) * vertices[2].textureCoord;
                rec.u = barycentricCoord.x;
                rec.v = barycentricCoord.y;
            }
            else
            {
                rec.u = u;
                rec.v = v;
            }
            if(vertices[0].tangent.x != -1.0f)
            {
                glm::vec2 deltaUV1 = vertices[2].textureCoord - vertices[0].textureCoord;
                glm::vec2 deltaUV2 = vertices[1].textureCoord - vertices[0].textureCoord;
                float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                glm::vec3 tangent(0.0f, 0.0f, 0.0f);
                glm::vec3 bitangent(0.0f, 0.0f, 0.0f);

                tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

                bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

                rec.tangent = tangent;
                rec.bitangent = bitangent;

                rec.tangent = u * vertices[0].tangent + v * vertices[1].tangent + (1 - u - v) * vertices[2].tangent;
                rec.bitangent = u * vertices[0].bitangent + v * vertices[1].bitangent + (1 - u - v) * vertices[2].bitangent;
            }
            else
            {
                rec.tangent = vertices[0].tangent;
                rec.bitangent = vertices[0].bitangent;
            }
            rec.modelMatrix = modelMatrix;
            rec.matPtr = matPtr;
            return true;
        }
        else // This means that there is a line intersection but not a ray intersection.
            return false;
    }

    virtual bool BoundingBox(AABB& outputBox) const
    {
        float xMin = infinity;
        float yMin = infinity;
        float zMin = infinity;
        float xMax = 1e-8;
        float yMax = 1e-8;
        float zMax = 1e-8;

        if (vertices[0].position.x < xMin)
            xMin = vertices[0].position.x;
        if (vertices[1].position.x < xMin)
            xMin = vertices[1].position.x;
        if (vertices[2].position.x < xMin)
            xMin = vertices[2].position.x;

        if (vertices[0].position.y < yMin)
            yMin = vertices[0].position.y;
        if (vertices[1].position.y < yMin)
            yMin = vertices[1].position.y;
        if (vertices[2].position.y < yMin)
            yMin = vertices[2].position.y;

        if (vertices[0].position.z < zMin)
            zMin = vertices[0].position.z;
        if (vertices[1].position.z < zMin)
            zMin = vertices[1].position.z;
        if (vertices[2].position.z < zMin)
            zMin = vertices[2].position.z;

        //MAX
        if (vertices[0].position.x > xMax)
            xMax = vertices[0].position.x;
        if (vertices[1].position.x > xMax)
            xMax = vertices[1].position.x;
        if (vertices[2].position.x > xMax)
            xMax = vertices[2].position.x;

        if (vertices[0].position.y > yMax)
            yMax = vertices[0].position.y;
        if (vertices[1].position.y > yMax)
            yMax = vertices[1].position.y;
        if (vertices[2].position.y > yMax)
            yMax = vertices[2].position.y;

        if (vertices[0].position.z > zMax)
            zMax = vertices[0].position.z;
        if (vertices[1].position.z > zMax)
            zMax = vertices[1].position.z;
        if (vertices[2].position.z > zMax)
            zMax = vertices[2].position.z;

        if (xMin == xMax)
        {
            xMin--;
            xMax++;
        }
        if (yMin == yMax)
        {
            yMin--;
            yMax++;
        }
        if (zMin == zMax)
        {
            zMin--;
            zMax++;
        }

        outputBox = AABB({ xMin, yMin, zMin }, { xMax, yMax, zMax });
        return true;
    }

public:
    Vertex vertices[3];
    glm::mat4 modelMatrix; //The model matrix of the mesh this triangle belongs to
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

    virtual bool BoundingBox(AABB& outputBox) const
    {
        outputBox = AABB(
            center - glm::vec3(radius, radius, radius),
            center + glm::vec3(radius, radius, radius)
        );
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

    virtual bool BoundingBox(AABB& outputBox) const
    {
        if (objects.empty()) return false;

        AABB tempBox;
        bool firstBox = true;

        for (const auto& object : objects)
        {
            if (!object->BoundingBox(tempBox)) return false;
            outputBox = firstBox ? tempBox : surroundingBox(outputBox, tempBox);
            firstBox = false;
        }

        return true;
    }

    std::vector<std::shared_ptr<Hittable>> objects;
};

class Box : public Hittable
{
public:
    Box() = default;
    Box(const glm::vec3& p0, const glm::vec3 p1, std::shared_ptr<Material> matPtr, glm::mat4 modelMatrix)
        : boxMin(p0), boxMax(p1)
    {
        Vertex vert0;
        Vertex vert1;
        Vertex vert2;
        vert0.normal = { 0.0f, 0.0f, 0.0f };
        vert1.normal = { 0.0f, 0.0f, 0.0f };
        vert2.normal = { 0.0f, 0.0f, 0.0f };

        vert0.position = glm::vec3(modelMatrix * glm::vec4(p0, 1.0f));
        vert1.position = glm::vec3(modelMatrix * glm::vec4(p0.x, p0.y, p1.z, 1.0f));
        vert2.position = glm::vec3(modelMatrix * glm::vec4(p0.x, p1.y, p0.z, 1.0f));
        sides.add(std::make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), matPtr, ""));
        vert0.position = glm::vec3(modelMatrix * glm::vec4(p0.x, p0.y, p1.z, 1.0f));
        vert1.position = glm::vec3(modelMatrix * glm::vec4(p0.x, p1.y, p1.z, 1.0f));
        vert2.position = glm::vec3(modelMatrix * glm::vec4(p0.x, p1.y, p0.z, 1.0f));
        sides.add(std::make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), matPtr, "")); //Left

        vert0.position = glm::vec3(modelMatrix * glm::vec4(p1.x, p0.y, p0.z, 1.0f));
        vert1.position = glm::vec3(modelMatrix * glm::vec4(p1.x, p0.y, p1.z, 1.0f));
        vert2.position = glm::vec3(modelMatrix * glm::vec4(p1.x, p1.y, p0.z, 1.0f));
        sides.add(std::make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), matPtr, ""));
        vert0.position = glm::vec3(modelMatrix * glm::vec4(p1.x, p0.y, p1.z, 1.0f));
        vert1.position = glm::vec3(modelMatrix * glm::vec4(p1, 1.0f));
        vert2.position = glm::vec3(modelMatrix * glm::vec4(p1.x, p1.y, p0.z, 1.0f));
        sides.add(std::make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), matPtr, "")); //Right

        vert0.position = glm::vec3(modelMatrix * glm::vec4(p0, 1.0f));
        vert1.position = glm::vec3(modelMatrix * glm::vec4(p1.x, p0.y, p0.z, 1.0f));
        vert2.position = glm::vec3(modelMatrix * glm::vec4(p0.x, p0.y, p1.z, 1.0f));
        sides.add(std::make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), matPtr, ""));
        vert0.position = glm::vec3(modelMatrix * glm::vec4(p1.x, p0.y, p0.z, 1.0f));
        vert1.position = glm::vec3(modelMatrix * glm::vec4(p1.x, p0.y, p1.z, 1.0f));
        vert2.position = glm::vec3(modelMatrix * glm::vec4(p0.x, p0.y, p1.z, 1.0f));
        sides.add(std::make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), matPtr, "")); //Floor

        vert0.position = glm::vec3(modelMatrix * glm::vec4(p0.x, p1.y, p0.z, 1.0f));
        vert1.position = glm::vec3(modelMatrix * glm::vec4(p1.x, p1.y, p0.z, 1.0f));
        vert2.position = glm::vec3(modelMatrix * glm::vec4(p0.x, p1.y, p1.z, 1.0f));
        sides.add(std::make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), matPtr, ""));
        vert0.position = glm::vec3(modelMatrix * glm::vec4(p1.x, p1.y, p0.z, 1.0f));
        vert1.position = glm::vec3(modelMatrix * glm::vec4(p1, 1.0f));
        vert2.position = glm::vec3(modelMatrix * glm::vec4(p0.x, p1.y, p1.z, 1.0f));
        sides.add(std::make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), matPtr, "")); //Top

        vert0.position = glm::vec3(modelMatrix * glm::vec4(p0, 1.0f));
        vert1.position = glm::vec3(modelMatrix * glm::vec4(p1.x, p0.y, p0.z, 1.0f));
        vert2.position = glm::vec3(modelMatrix * glm::vec4(p0.x, p1.y, p0.z, 1.0f));
        sides.add(std::make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), matPtr, ""));
        vert0.position = glm::vec3(modelMatrix * glm::vec4(p1.x, p0.y, p0.z, 1.0f));
        vert1.position = glm::vec3(modelMatrix * glm::vec4(p1.x, p1.y, p0.z, 1.0f));
        vert2.position = glm::vec3(modelMatrix * glm::vec4(p0.x, p1.y, p0.z, 1.0f));
        sides.add(std::make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), matPtr, "")); //Front

        vert0.position = glm::vec3(modelMatrix * glm::vec4(p1, 1.0f));
        vert1.position = glm::vec3(modelMatrix * glm::vec4(p0.x, p1.y, p1.z, 1.0f));
        vert2.position = glm::vec3(modelMatrix * glm::vec4(p0.x, p0.y, p1.z, 1.0f));
        sides.add(std::make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), matPtr, ""));
        vert0.position = glm::vec3(modelMatrix * glm::vec4(p1, 1.0f));
        vert1.position = glm::vec3(modelMatrix * glm::vec4(p0.x, p0.y, p1.z, 1.0f));
        vert2.position = glm::vec3(modelMatrix * glm::vec4(p1.x, p0.y, p1.z, 1.0f));
        sides.add(std::make_shared<Triangle>(vert0, vert1, vert2, glm::mat4(1.0f), matPtr, "")); //Back
    }

    virtual bool Hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const override
    {
        return sides.Hit(r, tMin, tMax, rec);
    }

    virtual bool BoundingBox(AABB& outputBox) const
    {
        outputBox = AABB(boxMin, boxMax);
        return true;
    }

public:
    glm::vec3 boxMin;
    glm::vec3 boxMax;
    HittableList sides;
};