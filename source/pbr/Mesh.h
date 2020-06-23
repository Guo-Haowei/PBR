#pragma once
#include "base/Definitions.h"

namespace pbr {

struct Vertex
{
    vec3 in_position;
    vec3 in_color;
};

static const Vertex g_triangle[] =
{
    { { 0.0f, 0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
    { { 0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
    { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
};

struct Mesh
{
    vector<vec3>        positions;
    vector<vec3>        normals;
    vector<vec2>        uvs;
    vector<uint32_t>    indices;
};

static Mesh createSphereMesh()
{
    Mesh sphere;
    auto& positions = sphere.positions;
    auto& uv = sphere.uvs;
    auto& normals = sphere.normals;
    auto& indices = sphere.indices;

    const unsigned int X_SEGMENTS = 64;
    const unsigned int Y_SEGMENTS = 64;
    const float PI = 3.14159265359;
    for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
    {
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = glm::cos(xSegment * 2.0f * PI) * glm::sin(ySegment * PI);
            float yPos = glm::cos(ySegment * PI);
            float zPos = glm::sin(xSegment * 2.0f * PI) * glm::sin(ySegment * PI);

            positions.push_back(vec3(xPos, yPos, zPos));
            uv.push_back(vec2(xSegment, ySegment));
            normals.push_back(vec3(xPos, yPos, zPos));
        }
    }

    bool oddRow = false;
    for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
    {
        if (!oddRow) // even rows: y == 0, y == 2; and so on
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                indices.push_back(y       * (X_SEGMENTS + 1) + x);
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
            }
        }
        else
        {
            for (int x = X_SEGMENTS; x >= 0; --x)
            {
                indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                indices.push_back(y       * (X_SEGMENTS + 1) + x);
            }
        }
        oddRow = !oddRow;
    }

    std::vector<float> data;
    for (unsigned int i = 0; i < positions.size(); ++i)
    {
        data.push_back(positions[i].x);
        data.push_back(positions[i].y);
        data.push_back(positions[i].z);
        if (uv.size() > 0)
        {
            data.push_back(uv[i].x);
            data.push_back(uv[i].y);
        }
        if (normals.size() > 0)
        {
            data.push_back(normals[i].x);
            data.push_back(normals[i].y);
            data.push_back(normals[i].z);
        }
    }

    return sphere;
}

} // namespace pbr
