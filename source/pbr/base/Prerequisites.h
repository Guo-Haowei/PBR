#pragma once
#include <array>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace pbr {
    using std::string;
    using std::wstring;
    using std::array;
    using std::vector;
    using std::unique_ptr;
    using std::shared_ptr;
    using std::ostream;
    using std::cout;
    using std::wcout;
    using std::endl;

    using glm::vec2;
    using glm::vec3;
    using glm::vec4;
    using glm::mat4;

    static const float Pi = glm::pi<float>();
    static const float QuarterPi = 0.25f * Pi;
    static const float HalfPi = 0.5f * Pi;
    static const float TwoPi = 2.0f * Pi;
} // namespace pbr