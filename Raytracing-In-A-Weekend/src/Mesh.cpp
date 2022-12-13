#include <iostream>
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"
#include "Bvh.h"

Mesh::Mesh(glm::mat4 model, std::string const& location)
    : modelMatrix(model), matPtr(nullptr), directory(location.substr(0, location.find_last_of('/')))
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(location, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);

    if (scene)
    {
        float xMin = infinity;
        float yMin = infinity;
        float zMin = infinity;
        float xMax = 1e-8;
        float yMax = 1e-8;
        float zMax = 1e-8;

        processNode(scene->mRootNode, scene, xMin, yMin, zMin, xMax, yMax, zMax);

        glm::vec3 minimum = glm::vec3(xMin, yMin, zMin);
        glm::vec3 maximum = glm::vec3(xMax, yMax, zMax);
        boundingBox = std::make_shared<AABB>(minimum, maximum);

        std::shared_ptr<BVHNode> bvh = std::make_shared<BVHNode>(triangles, 0, triangles.size(), 10);
        triangles.clear();
        triangles.push_back(bvh);
    }
    else
    {
        std::cout << "Could not import model at location: " << location << std::endl;
    }
}

void Mesh::processNode(aiNode* node, const aiScene* scene, float& xMin, float& yMin, float& zMin, float& xMax, float& yMax, float& zMax)
{
    
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene, xMin, yMin, zMin, xMax, yMax, zMax);
    }
    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, xMin, yMin, zMin, xMax, yMax, zMax);
    }
}

void Mesh::processMesh(aiMesh* mesh, const aiScene* scene, float& xMin, float& yMin, float& zMin, float& xMax, float& yMax, float& zMax)
{
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    std::shared_ptr<Texture> diffuseTexture = nullptr;
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
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
    if (material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0 && matPtr)
    {
        std::shared_ptr<Texture> roughnessTexture = nullptr;
        aiString str;
        material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &str);

        std::string filename = directory + '/' + str.C_Str();
        if (std::find(loadedTextures.begin(), loadedTextures.end(), filename) == loadedTextures.end())
        {
            loadedTextures.push_back(filename);
            roughnessTexture = std::make_shared<Texture>(filename.c_str());
        }

        if(roughnessTexture)
            std::static_pointer_cast<PBRMaterial, Material>(matPtr)->setRoughnessTexture(roughnessTexture);
    }
    if (material->GetTextureCount(aiTextureType_NORMALS) > 0 && matPtr)
    {
        std::shared_ptr<Texture> normalTexture = nullptr;
        aiString str;
        material->GetTexture(aiTextureType_NORMALS, 0, &str);

        std::string filename = directory + '/' + str.C_Str();
        if (std::find(loadedTextures.begin(), loadedTextures.end(), filename) == loadedTextures.end())
        {
            loadedTextures.push_back(filename);
            normalTexture = std::make_shared<Texture>(filename.c_str());
        }

        if (normalTexture)
            std::static_pointer_cast<PBRMaterial, Material>(matPtr)->setNormalTexture(normalTexture);
    }

    for (unsigned int f = 0; f < mesh->mNumFaces; f++)
    {
        aiFace face = mesh->mFaces[f];
        unsigned int v0 = face.mIndices[0];
        unsigned int v1 = face.mIndices[1];
        unsigned int v2 = face.mIndices[2];
        glm::vec3 pos0(mesh->mVertices[v0].x, mesh->mVertices[v0].y, mesh->mVertices[v0].z);
        glm::vec3 pos1(mesh->mVertices[v1].x, mesh->mVertices[v1].y, mesh->mVertices[v1].z);
        glm::vec3 pos2(mesh->mVertices[v2].x, mesh->mVertices[v2].y, mesh->mVertices[v2].z);
        glm::vec3 tangent0(mesh->mTangents[v0].x, mesh->mTangents[v0].y, mesh->mTangents[v0].z);
        glm::vec3 tangent1(mesh->mTangents[v1].x, mesh->mTangents[v1].y, mesh->mTangents[v1].z);
        glm::vec3 tangent2(mesh->mTangents[v2].x, mesh->mTangents[v2].y, mesh->mTangents[v2].z);
        glm::vec3 bitangent0(mesh->mBitangents[v0].x, mesh->mBitangents[v0].y, mesh->mBitangents[v0].z);
        glm::vec3 bitangent1(mesh->mBitangents[v1].x, mesh->mBitangents[v1].y, mesh->mBitangents[v1].z);
        glm::vec3 bitangent2(mesh->mBitangents[v2].x, mesh->mBitangents[v2].y, mesh->mBitangents[v2].z);

        glm::vec3 tex0(0.0f, 0.0f, 0.0f);
        glm::vec3 tex1(0.1f, 0.0f, 0.0f);
        glm::vec3 tex2(0.5f, 1.0f, 0.0f);
        if (mesh->mTextureCoords[0])
        {
            tex0.x = mesh->mTextureCoords[0][v0].x;
            tex0.y = mesh->mTextureCoords[0][v0].y;
            tex1.x = mesh->mTextureCoords[0][v1].x;
            tex1.y = mesh->mTextureCoords[0][v1].y;
            tex2.x = mesh->mTextureCoords[0][v2].x;
            tex2.y = mesh->mTextureCoords[0][v2].y;
        }
        glm::vec3 norm0(0.0f, 0.0f, 0.0f);
        glm::vec3 norm1(0.0f, 0.0f, 0.0f);
        glm::vec3 norm2(0.0f, 0.0f, 0.0f);
        if (mesh->HasNormals())
        {
            norm0 = glm::normalize(glm::vec3(mesh->mNormals[v0].x, mesh->mNormals[v0].y, mesh->mNormals[v0].z));
            norm1 = glm::normalize(glm::vec3(mesh->mNormals[v1].x, mesh->mNormals[v1].y, mesh->mNormals[v1].z));
            norm2 = glm::normalize(glm::vec3(mesh->mNormals[v2].x, mesh->mNormals[v2].y, mesh->mNormals[v2].z));
        }

        glm::mat3 normalMatrix(glm::transpose(glm::inverse(modelMatrix)));
        pos0 = glm::vec3(modelMatrix * glm::vec4(pos0, 1.0f));
        pos1 = glm::vec3(modelMatrix * glm::vec4(pos1, 1.0f));
        pos2 = glm::vec3(modelMatrix * glm::vec4(pos2, 1.0f));
        norm0 = glm::vec3(normalMatrix * glm::vec4(norm0, 1.0f));
        norm1 = glm::vec3(normalMatrix * glm::vec4(norm1, 1.0f));
        norm2 = glm::vec3(normalMatrix * glm::vec4(norm2, 1.0f));
        tangent0 = glm::vec3(modelMatrix * glm::vec4(tangent0, 1.0f));
        tangent1 = glm::vec3(modelMatrix * glm::vec4(tangent1, 1.0f));
        tangent2 = glm::vec3(modelMatrix * glm::vec4(tangent2, 1.0f));
        bitangent0 = glm::vec3(modelMatrix * glm::vec4(bitangent0, 1.0f));
        bitangent1 = glm::vec3(modelMatrix * glm::vec4(bitangent1, 1.0f));
        bitangent2 = glm::vec3(modelMatrix * glm::vec4(bitangent2, 1.0f));

        Vertex vert0(pos0, norm0, tex0, tangent0, bitangent0);
        Vertex vert1(pos1, norm1, tex1, tangent1, bitangent1);
        Vertex vert2(pos2, norm2, tex2, tangent2, bitangent2);

        auto tri = std::make_shared<Triangle>(vert0, vert1, vert2, modelMatrix, matPtr, "");
        triangles.push_back(tri);

        //MIN
        if (pos0.x < xMin)
            xMin = pos0.x;
        if (pos1.x < xMin)
            xMin = pos1.x;
        if (pos2.x < xMin)
            xMin = pos2.x;

        if (pos0.y < yMin)
            yMin = pos0.y;
        if (pos1.y < yMin)
            yMin = pos1.y;
        if (pos2.y < yMin)
            yMin = pos2.y;

        if (pos0.z < zMin)
            zMin = pos0.z;
        if (pos1.z < zMin)
            zMin = pos1.z;
        if (pos2.z < zMin)
            zMin = pos2.z;

        //MAX
        if (pos0.x > xMax)
            xMax = pos0.x;
        if (pos1.x > xMax)
            xMax = pos1.x;
        if (pos2.x > xMax)
            xMax = pos2.x;

        if (pos0.y > yMax)
            yMax = pos0.y;
        if (pos1.y > yMax)
            yMax = pos1.y;
        if (pos2.y > yMax)
            yMax = pos2.y;

        if (pos0.z > zMax)
            zMax = pos0.z;
        if (pos1.z > zMax)
            zMax = pos1.z;
        if (pos2.z > zMax)
            zMax = pos2.z;
    }

}
