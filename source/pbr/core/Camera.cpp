#include "Camera.h"
#include "Window.h"
#include "Utility.h"
#include <glm/gtx/vector_angle.hpp>

namespace pbr {

Camera::Camera(float fov, float aspect, float zNear, float zFar)
    : m_fov(fov), m_aspect(aspect), m_zNear(zNear), m_zFar(zFar), m_transform(mat4(1.0f)), m_dirty(true)
{
}

mat4 Camera::ViewMatrix(const mat4& transform)
{
    const vec4& position = transform[3];
    const vec4& u = transform[0];
    const vec4& v = transform[1];
    const vec4& w = transform[2];
    const vec3 eye(position);
    const vec3 center(position - w);
    const vec3 up(v);
    return glm::lookAtRH(eye, center, up);
}

mat4 Camera::ViewMatrix() const
{
    return Camera::ViewMatrix(m_transform);
}

mat4 Camera::ProjectionMatrixD3d() const
{
    return mat4({1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 0.5, 0}, {0, 0, 0, 1}) *
           mat4({1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 1, 1}) *
           ProjectionMatrixGl();
}

mat4 Camera::ProjectionMatrixGl() const
{
    return glm::perspective(m_fov, m_aspect, m_zNear, m_zFar);
}

void CubeCamera::ViewMatricesGl(array<mat4, 6>& inMatrices) const
{
    inMatrices = {
        glm::lookAt(vec3(0), vec3( 1,  0,  0), vec3(0, -1,  0)),
        glm::lookAt(vec3(0), vec3(-1,  0,  0), vec3(0, -1,  0)),
        glm::lookAt(vec3(0), vec3( 0,  1,  0), vec3(0,  0,  1)),
        glm::lookAt(vec3(0), vec3( 0, -1,  0), vec3(0,  0, -1)),
        glm::lookAt(vec3(0), vec3( 0,  0,  1), vec3(0, -1,  0)),
        glm::lookAt(vec3(0), vec3( 0,  0, -1), vec3(0, -1,  0)),
    };
}

void CubeCamera::ViewMatricesD3d(array<mat4, 6>& inMatrices) const
{
    inMatrices = {
        glm::lookAt(vec3(0), vec3( 1,  0,  0), vec3(0, -1,  0)), // right
        glm::lookAt(vec3(0), vec3(-1,  0,  0), vec3(0, -1,  0)),
        glm::lookAt(vec3(0), vec3( 0, -1,  0), vec3(0,  0, -1)),
        glm::lookAt(vec3(0), vec3( 0,  1,  0), vec3(0,  0,  1)),
        glm::lookAt(vec3(0), vec3( 0,  0,  1), vec3(0, -1,  0)),
        glm::lookAt(vec3(0), vec3( 0,  0, -1), vec3(0, -1,  0)),
    };
}

CameraController::CameraController(Camera* pCamera)
    : m_pCamera(pCamera), m_dirty(false)
{
}

vec3 CameraController::virtualPoint(const Extent2i& extent, const vec2& cursorPos)
{
    vec2 coord = vec2(cursorPos.x / extent.width, cursorPos.y / extent.height);
    coord = (coord - 0.5f) * vec2(2.0f, -2.0f);
    vec2 normalizedCoord = glm::normalize(coord); // x in [-1, 1], y in [-1, 1], x^2 + y^2 in [0, 2]
    return vec3
    {
        coord.x / std::sqrtf(2.0f),
        coord.y / std::sqrtf(2.0f),
        std::sqrtf(1.0f - 0.5f * (coord.x * coord.x + coord.y * coord.y))
    };
}

void CameraController::Update(const Window* pWindow)
{
    m_pCamera->m_dirty = false;

    // handle projection matrix change
    const float aspect = pWindow->GetAspectRatio();
    if (aspect != m_pCamera->m_aspect)
    {
        m_pCamera->m_aspect = aspect;
        m_pCamera->m_dirty = true;
    }

    // zoom
    mat4& M = m_pCamera->m_transform;
    const double scroll = pWindow->GetScroll();
    if (scroll != 0)
    {
        float deltaZ = static_cast<float>(scroll) * 0.6f;
        M = glm::translate(glm::mat4(1.0f), deltaZ * vec3(M[2])) * M;
        m_pCamera->m_dirty = true;
        // zooming should change center
        return;
    }

    // rotate
    const vec2& lastFrameCursorPos = pWindow->GetLastFrameCursorPos();
    const vec2& thisFrameCursorPos = pWindow->GetThisFrameCursorPos();
    const vec2 cursorDelta = thisFrameCursorPos - lastFrameCursorPos;
    const Extent2i& windowExtent = pWindow->GetWindowExtent();

    const float tolerance = 0.001f;
    if (glm::abs(cursorDelta.x) < tolerance && glm::abs(cursorDelta.y) < tolerance)
        return;

    if (pWindow->IsButtonDown(Window::BUTTON_LEFT))
    {
        // rotate
        vec3 p0 = virtualPoint(windowExtent, lastFrameCursorPos);
        vec3 p1 = virtualPoint(windowExtent, thisFrameCursorPos);
        vec3 axis = glm::cross(p1, p0);
        float angle = glm::orientedAngle(p1, p0, axis);
        mat4 R = glm::rotate(mat4(1.0f), angle, axis);
        // rotate around center
        if (!utility::IsNaN(R))
        {
            mat4 T_0 = glm::translate(mat4(1.0f), -center);
            mat4 T_1 = glm::translate(mat4(1.0f),  center);
            M = T_1 * R * T_0 * M;
            m_pCamera->m_dirty = true;
        }
    }
    else if (pWindow->IsButtonDown(Window::BUTTON_RIGHT))
    {
        // move center
        vec3 up(M[1]);
        vec3 right(M[0]);
        vec3 pan = right * (-cursorDelta.x / (float)windowExtent.width) + up * (cursorDelta.y / (float)windowExtent.height);
        pan *= 10.0f;
        center += pan;
        mat4 translation = glm::translate(mat4(1.0f), pan);
        M = translation * M;
        m_pCamera->m_dirty = true;
    }
}

} // namespace pbr

