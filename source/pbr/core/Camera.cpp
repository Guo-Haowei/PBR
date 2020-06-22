#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace pbr {

Camera::Camera(float fov, float aspect, float zNear, float zFar)
    : m_fov(fov), m_aspect(aspect), m_zNear(zNear), m_zFar(zFar), m_transform(mat4(1.0f))
{
}

mat4 Camera::projectionMatrix() const
{
    return glm::perspective(m_fov, m_aspect, m_zNear, m_zFar);
}

mat4 Camera::viewMatrix() const
{
    const vec4& position = m_transform[3];
    const vec4& u = m_transform[0];
    const vec4& v = m_transform[1];
    const vec4& w = m_transform[2];
    const vec3 eye(position);
    const vec3 center(position - w);
    const vec3 up(v);
    return glm::lookAt(eye, center, up);
}

} // namespace pbr

