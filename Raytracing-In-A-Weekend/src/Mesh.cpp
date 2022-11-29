#include "Mesh.h"
#include "Texture.h"
#include "Material.h"

Mesh::Mesh(Vec3 pos, std::string const& location)
    : position(pos), matPtr(nullptr), directory(location.substr(0, location.find_last_of('/')))
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
        processMesh(mesh, scene);
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

void Mesh::processMesh(aiMesh* mesh, const aiScene* scene)
{
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        std::shared_ptr<Texture> diffuseTexture = nullptr;
        aiString str;
        material->GetTexture(aiTextureType_DIFFUSE, 0, &str);

        std::string filename = directory + '/' + str.C_Str();
        if(std::find(loadedTextures.begin(), loadedTextures.end(), filename) == loadedTextures.end())
        {
            loadedTextures.push_back(filename);
            diffuseTexture = std::make_shared<Texture>(filename.c_str());
        }

        if(!matPtr)
            matPtr = std::make_shared<PBRMaterial>(diffuseTexture);
    }

    for (unsigned int f = 0; f < mesh->mNumFaces; f++)
    {
        aiFace face = mesh->mFaces[f];
        unsigned int v0 = face.mIndices[0];
        unsigned int v1 = face.mIndices[1];
        unsigned int v2 = face.mIndices[2];
        Vec3 pos0(position.x + mesh->mVertices[v0].x, position.y + mesh->mVertices[v0].y, position.z + mesh->mVertices[v0].z);
        Vec3 pos1(position.x + mesh->mVertices[v1].x, position.y + mesh->mVertices[v1].y, position.z + mesh->mVertices[v1].z);
        Vec3 pos2(position.x + mesh->mVertices[v2].x, position.y + mesh->mVertices[v2].y, position.z + mesh->mVertices[v2].z);

        Vec3 tex0(0.0, 0.0, 0.0);
        Vec3 tex1(0.1, 0.0, 0.0);
        Vec3 tex2(0.5, 1.0, 0.0);
        if (mesh->mTextureCoords[0])
        {
            tex0.x = mesh->mTextureCoords[0][v0].x;
            tex0.y = mesh->mTextureCoords[0][v0].y;
            tex1.x = mesh->mTextureCoords[0][v1].x;
            tex1.y = mesh->mTextureCoords[0][v1].y;
            tex2.x = mesh->mTextureCoords[0][v2].x;
            tex2.y = mesh->mTextureCoords[0][v2].y;
        }
        Triangle tri(pos0, pos1, pos2, tex0, tex1, tex2, matPtr, "");
        triangles.push_back(tri);
    }

}
