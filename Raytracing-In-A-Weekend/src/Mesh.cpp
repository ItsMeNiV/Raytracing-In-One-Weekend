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
        double xMin = infinity;
        double yMin = infinity;
        double zMin = infinity;
        double xMax = 1e-8;
        double yMax = 1e-8;
        double zMax = 1e-8;

        processNode(scene->mRootNode, scene, xMin, yMin, zMin, xMax, yMax, zMax);

        Vec3 minimum = Vec3(xMin, yMin, zMin);
        Vec3 maximum = Vec3(xMax, yMax, zMax);
        boundingBox = std::make_shared<AABB>(minimum, maximum);
    }
    else
    {
        std::cout << "Could not import model at location: " << location << std::endl;
    }
}

void Mesh::processNode(aiNode* node, const aiScene* scene, double& xMin, double& yMin, double& zMin, double& xMax, double& yMax, double& zMax)
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

void Mesh::processMesh(aiMesh* mesh, const aiScene* scene, double& xMin, double& yMin, double& zMin, double& xMax, double& yMax, double& zMax)
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
