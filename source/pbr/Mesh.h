#pragma once
#include "base/Prerequisites.h"

namespace pbr {

struct VertexPosColor
{
    vec3 in_position;
    vec3 in_color;
};

struct Vertex
{
    vec3 position;
    vec2 uv;
    vec3 normal;

    Vertex(const vec3& pos, const vec2& uv, const vec3& normal)
        : position(pos), uv(uv), normal(normal)
    {
    }
};

struct Mesh
{
    vector<Vertex> vertices;
    vector<uvec3>  indices;
};

struct Cube
{
    vector<vec3> vertices;
    vector<uvec3>  indices;
};

extern Cube CreateCubeMesh(float scale = 1.0f);

extern Mesh CreateSphereMesh(float radius = 1.0f, uint32_t widthSegment = 32, uint32_t heightSegment = 32);

} // namespace pbr
