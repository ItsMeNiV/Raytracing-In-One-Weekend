#pragma once
#include <vector>
#include "Hittable.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

class Mesh : public Hittable
{
public:
	Mesh(Vec3 pos, const char* location, std::shared_ptr<Material> matPtr);

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
    std::shared_ptr<Material> matPtr;

    void processNode(aiNode* node, const aiScene* scene);
    void processMesh(aiMesh* mesh);

};