#include "Mesh.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

Mesh::Mesh(Vec3 pos, const char* location)
    : position(pos)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(location, aiProcess_Triangulate);

    if (!scene)
        std::cout << "Could not import model at location: " << location << std::endl;
}