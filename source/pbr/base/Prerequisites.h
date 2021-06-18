#pragma once
#include <array>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#ifdef min
#undef min
#endif  // min
#ifdef max
#undef max
#endif  // max

namespace pbr {
using std::array;
using std::cout;
using std::endl;
using std::optional;
using std::ostream;
using std::set;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;
using std::wcout;
using std::wstring;

using glm::vec2;
using glm::vec3;
using glm::vec4;

using glm::uvec3;

using glm::mat4;

static const float Pi = glm::pi<float>();
static const float QuarterPi = 0.25f * Pi;
static const float HalfPi = 0.5f * Pi;
static const float TwoPi = 2.0f * Pi;
}  // namespace pbr