#include "Camera.h"
#include "Window.h"
#include <glm/gtc/matrix_transform.hpp>

namespace pbr {

Camera::Camera(float fov, float aspect, float zNear, float zFar)
    : m_fov(fov), m_aspect(aspect), m_zNear(zNear), m_zFar(zFar), m_transform(mat4(1.0f)), m_dirty(true)
{
}

mat4 Camera::ProjectionMatrix() const
{
    return glm::perspective(m_fov, m_aspect, m_zNear, m_zFar);
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
    return glm::lookAt(eye, center, up);
}

CameraController::CameraController(Camera* pCamera)
    : m_pCamera(pCamera), m_dirty(false)
{
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

    // handle view matrix change
    const vec2 lastFrameCursorPos = pWindow->GetLastFrameCursorPos();
    const vec2 thisFrameCursorPos = pWindow->GetThisFrameCursorPos();
    const vec2 cursorDelta = thisFrameCursorPos - lastFrameCursorPos;
    const double scroll = pWindow->GetScroll();

    mat4& M = m_pCamera->m_transform;

    // rotate around center
    if (pWindow->IsButtonDown(GLFW_MOUSE_BUTTON_LEFT))
    {
        cout << "left button down" << endl;
    }
    // pan
    else if (pWindow->IsButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
    {
        cout << "right button down" << endl;
    }
    else if (scroll != 0)
    {
        float deltaZ = static_cast<float>(scroll) * 0.1f;
        M = glm::translate(glm::mat4(1.0f), deltaZ * vec3(M[2])) * M;
        m_pCamera->m_dirty = true;
    }
}

} // namespace pbr

