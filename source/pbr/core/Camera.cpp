#include "Camera.h"
#include "Window.h"
#include "Utility.h"

namespace pbr {

Camera::Camera(float fov, float aspect, float zNear, float zFar)
    : m_fov(fov), m_aspect(aspect), m_zNear(zNear), m_zFar(zFar), m_transform(mat4(1.0f)), m_dirty(true)
{
}

mat4 Camera::ViewMatrix() const
{
    const vec4& position = m_transform[3];
    const vec4& u = m_transform[0];
    const vec4& v = m_transform[1];
    const vec4& w = m_transform[2];
    const vec3 eye(position);
    const vec3 center(position - w);
    const vec3 up(v);
    return glm::lookAtRH(eye, center, up);
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

CameraController::CameraController(Camera* pCamera)
    : m_pCamera(pCamera), m_dirty(false)
{
}

bool CameraController::virtualPoint(const Extent2i& extent, const vec2& cursorPos, vec3& point)
{
    vec2 coord = vec2(cursorPos.x / extent.width, cursorPos.y / extent.height);
    coord = (coord - 0.5f) * vec2(2.0f, -2.0f);
    vec2 normalizedCoord = glm::normalize(coord); // x in [-1, 1], y in [-1, 1], x^2 + y^2 < 1
    if (extent.width > extent.height)
        coord.x = coord.x * extent.width / extent.height;
    else
        coord.y = coord.y * extent.height / extent.width;

    float length2D = coord.x * coord.x + coord.y * coord.y;
    bool insideTrackball = length2D < 1.0f;

    if (!insideTrackball)
    {
        point.x = normalizedCoord.x;
        point.y = normalizedCoord.y;
        point.z = 0.0f; // set it to 0 so that it doesn't affect dot product in 2D space
        return false;
    }
    else
    {
        point.x = coord.x;
        point.y = coord.y;
        point.z = glm::sqrt(1.0f - length2D);
        return true;
    }
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

    // scrolling
    mat4& M = m_pCamera->m_transform;
    const double scroll = pWindow->GetScroll();
    if (scroll != 0)
    {
        float deltaZ = static_cast<float>(scroll) * 0.6f;
        M = glm::translate(glm::mat4(1.0f), deltaZ * vec3(M[2])) * M;
        m_pCamera->m_dirty = true;
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

    vec3 p0, p1;
    bool p0InTrackball = virtualPoint(windowExtent, lastFrameCursorPos, p0);
    bool p1InTrackball = virtualPoint(windowExtent, thisFrameCursorPos, p1);
    if (pWindow->IsButtonDown(Window::BUTTON_LEFT))
    {
        // trackball rotation
        if (p0InTrackball && p1InTrackball)
        {
            vec3 axis = glm::cross(p1, p0);
            float angle = glm::acos(glm::dot(p0, p1));
            mat4 R = glm::rotate(mat4(1.0f), angle, axis);
            if (!utility::IsNaN(M))
            {
                M = R * M;
                m_pCamera->m_dirty = true;
            }
        }
        // rotate around view
        else if (!p0InTrackball && !p1InTrackball)
        {
            vec3 outVector = glm::cross(p1, p0);
            float angle = glm::sign(glm::dot(outVector, vec3(0, 0, 1))) * glm::acos(glm::dot(p0, p1));
            vec3 axis = vec3(M[2]);
            mat4 R = glm::rotate(mat4(1.0f), angle, axis);
            if (!utility::IsNaN(M))
            {
                M = R * M;
                m_pCamera->m_dirty = true;
            }
        }
    }
    // track ball rotation around center
}

} // namespace pbr

