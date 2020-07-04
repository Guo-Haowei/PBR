#pragma once
#include "Mesh.h"

namespace pbr {

class AssimpLoader
{
public:
    TexturedMesh load(const char* path);
private:
};

} // namespace pbr
