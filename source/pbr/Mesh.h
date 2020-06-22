#pragma once
#include "Definitions.h"

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

} // namespace pbr
