#include "AssimpLoader.h"
#include <assert.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include "base/Error.h"

using namespace std;

namespace pbr {

TexturedMesh AssimpLoader::load(const char* path) {
    string fullpath(path);
    Assimp::Importer importer;
    cout << "[Log] Loading scene [" << fullpath << "]" << endl;
    const aiScene* aiscene = importer.ReadFile(fullpath,
                                               aiProcess_Triangulate |
                                                   aiProcess_FlipUVs |
                                                   aiProcess_CalcTangentSpace |
                                                   // aiProcess_GenSmoothNormals |
                                                   aiProcess_JoinIdenticalVertices);

    if (!aiscene || aiscene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiscene->mRootNode)  // if is Not Zero
    {
        std::cout << "[ERROR] assimp failed to load.";
        std::cout << importer.GetErrorString() << std::endl;
        throw std::runtime_error("Failed to parse scene " + fullpath);
    }

    assert(aiscene->mNumMeshes);

    TexturedMesh mesh;

    aiMesh* aimesh = aiscene->mMeshes[0];
    for (int i = 0; i < aimesh->mNumVertices; ++i) {
        const auto& position = aimesh->mVertices[i];
        TexturedVertex vertex;
        vertex.position = vec3(position.x, position.y, position.z);
        const auto& normal = aimesh->mNormals[i];
        vertex.normal = vec3(normal.x, normal.y, normal.z);
        const auto& uv = aimesh->mTextureCoords[0][i];
        vertex.uv = vec2(uv.x, uv.y);
        const auto& tangent = aimesh->mTangents[i];
        vertex.tangent = vec3(tangent.x, tangent.y, tangent.z);
        const auto& bitangent = aimesh->mBitangents[i];
        vertex.bitangent = vec3(bitangent.x, bitangent.y, bitangent.z);
        mesh.vertices.push_back(vertex);
    }

    for (int i = 0; i < aimesh->mNumFaces; ++i) {
        const aiFace& face = aimesh->mFaces[i];
        mesh.indices.push_back(uvec3(face.mIndices[1], face.mIndices[0], face.mIndices[2]));
    }

    std::cout << "****************************************\n";
    std::cout << "size of indices: " << mesh.indices.size() * sizeof(uvec3) << std::endl;
    std::cout << "size of vertices: " << mesh.vertices.size() * sizeof(TexturedVertex) << std::endl;

    std::ofstream bin("test.bin", ios::out | ios::binary);
    if (!bin.is_open())
        throw runtime_error("Failed to open for write");

    bin.write((char*)mesh.indices.data(), mesh.indices.size() * sizeof(uvec3));
    bin.write((char*)mesh.vertices.data(), mesh.vertices.size() * sizeof(TexturedVertex));
    bin.close();

    return mesh;
}

}  // namespace pbr

int main() {
    pbr::AssimpLoader loader;
    try {
        loader.load("gltf/WaterBottle.gltf");
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << '\n';
    }
}
