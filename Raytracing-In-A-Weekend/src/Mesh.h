#pragma once
#include <vector>
#include "Hittable.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

class Mesh : public Hittable
{
public:
	Mesh(Vec3 pos, std::string const& location);

    virtual bool Hit(const Ray& r, double tMin, double tMax, HitRecord& rec) const override
    {
        if (!boundingBox->Hit(r, tMin, tMax))
            return false;

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
    std::shared_ptr<AABB> boundingBox;
	Vec3 position;
	std::vector<Triangle> triangles;
    std::shared_ptr<Material> matPtr;
    std::string directory;
    std::vector<std::string> loadedTextures;

    void processNode(aiNode* node, const aiScene* scene, double& xMin, double& yMin, double& zMin, double& xMax, double& yMax, double& zMax);
    void processMesh(aiMesh* mesh, const aiScene* scene, double& xMin, double& yMin, double& zMin, double& xMax, double& yMax, double& zMax);

};