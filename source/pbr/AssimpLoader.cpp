#include "ModelLoader.h"
#include "base/Error.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <iostream>
#include <stdexcept>
#include <assert.h>
#include <fstream>
using std::string;
using std::ifstream;

namespace pbr {

Mesh ModelLoader::load(const char* path)
{
    string fullpath(path);
    Assimp::Importer importer;
    cout << "[Log] Loading scene [" << fullpath << "]" << endl;
    const aiScene* aiscene = importer.ReadFile(fullpath,
        aiProcess_Triangulate |
        // aiProcess_FlipUVs |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices
    );

    if(!aiscene || aiscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiscene->mRootNode) // if is Not Zero
    {
        std::cout << "[ERROR] assimp failed to load.";
        std::cout << importer.GetErrorString() << std::endl;
        throw std::runtime_error("Failed to parse scene " + fullpath);
    }

    assert(aiscene->mNumMeshes);

    Mesh mesh;

    aiMesh* aimesh = aiscene->mMeshes[0];
    for (int i = 0; i < aimesh->mNumVertices; ++i)
    {
        const auto& position = aimesh->mVertices[i];
        Vertex vertex;
        vertex.position = vec3(position.x, position.y, position.z);
        const auto& normal = aimesh->mNormals[i];
        vertex.normal = vec3(normal.x, normal.y, normal.z);
        const auto& uv = aimesh->mTextureCoords[0][i];
        vertex.uv = vec2(uv.x, uv.y);
        mesh.vertices.push_back(vertex);
    }

    for (int i = 0; i < aimesh->mNumFaces; ++i)
    {
        const aiFace& face = aimesh->mFaces[i];
        mesh.indices.push_back(uvec3(face.mIndices[1], face.mIndices[0], face.mIndices[2]));
    }

    return mesh;
}

} // namespace pbr
