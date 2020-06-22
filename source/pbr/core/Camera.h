#include "base/Prerequisites.h"

namespace pbr {

    class Camera
    {
    public:
        Camera(float fov = QuarterPi, float aspect = 1.0f, float zNear = 0.1f, float zFar = 100.0f);
        mat4 projectionMatrix() const;
        mat4 viewMatrix() const;
        inline void setTransformation(const mat4& transform) { m_transform = transform; }
        inline void setAspect(float aspect) { m_aspect = aspect; }
    private:
        mat4    m_transform;
        float   m_fov;
        float   m_aspect;
        float   m_zNear;
        float   m_zFar;
    };

} // namespace pbr
