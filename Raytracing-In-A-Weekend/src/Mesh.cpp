#include "Mesh.h"

Mesh::Mesh(Vec3 pos, const char* location, std::shared_ptr<Material> matPtr)
    : position(pos), matPtr(matPtr)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(location, aiProcess_Triangulate);

    if (scene)
    {
        processNode(scene->mRootNode, scene);
    }
    else
    {
        std::cout << "Could not import model at location: " << location << std::endl;
    }
}

void Mesh::processNode(aiNode* node, const aiScene* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh);
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

void Mesh::processMesh(aiMesh* mesh)
{
    for (unsigned int i = 0; i < mesh->mNumVertices; i+=3)
    {
        Vec3 pos0(position.x + mesh->mVertices[i].x, position.y + mesh->mVertices[i].y, position.z + mesh->mVertices[i].z);
        Vec3 pos1(position.x + mesh->mVertices[i+1].x, position.y + mesh->mVertices[i+1].y, position.z + mesh->mVertices[i+1].z);
        Vec3 pos2(position.x + mesh->mVertices[i+2].x, position.y + mesh->mVertices[i+2].y, position.z + mesh->mVertices[i+2].z);
        Triangle tri(pos0, pos1, pos2, matPtr, "");
        triangles.push_back(tri);
    }
}
