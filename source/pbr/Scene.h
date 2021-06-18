#pragma once
#include <cstddef>  // offsetof
#include "base/Prerequisites.h"

namespace pbr {

struct Light {
   public:
    Light() {}
    Light(const vec3& position, const vec3& color)
        : position(position), color(color) {}
    vec3 position;

   private:
    float _padding0;

   public:
    vec3 color;

   private:
    float _padding1;
};

static_assert(offsetof(Light, color) == sizeof(vec4));
static_assert(sizeof(Light) == 2 * sizeof(vec4));

static const array<Light, 4> g_lights = {
    Light(10.0f * vec3(-1, +1, +1), vec3(300)),
    Light(10.0f * vec3(+1, +1, +1), vec3(300)),
    Light(10.0f * vec3(-1, -1, +1), vec3(300)),
    Light(10.0f * vec3(+1, -1, +1), vec3(300))
};

}  // namespace pbr
