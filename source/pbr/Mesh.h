#pragma once
#include "base/Prerequisites.h"

namespace pbr {

struct VertexPosColor
{
    vec3 in_position;
    vec3 in_color;
};

static const VertexPosColor g_triangle[] =
{
    { { 0.0f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
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

extern Mesh createSphereMesh(float radius = 1.0f, uint32_t widthSegment = 32, uint32_t heightSegment = 32);

static const Mesh g_sphere = createSphereMesh();

} // namespace pbr
