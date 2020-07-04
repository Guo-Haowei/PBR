#pragma once
#include "Mesh.h"

namespace pbr {

class ModelLoader
{
public:
    Mesh load(const char* path);
private:
};

} // namespace pbr
