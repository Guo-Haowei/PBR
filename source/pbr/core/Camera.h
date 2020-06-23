#include "base/Prerequisites.h"

namespace pbr {

    class Window;

    class Camera
    {
    public:
        Camera(float fov = QuarterPi, float aspect = 1.0f, float zNear = 0.1f, float zFar = 100.0f);
        mat4 ProjectionMatrix() const;
        mat4 ViewMatrix() const;
        inline void SetTransformation(const mat4& transform) { m_transform = transform; m_dirty = true; }
        inline void SetAspect(float aspect) { m_aspect = aspect; m_dirty = true; }
        inline bool IsDirty() const { return m_dirty; }
    private:
        mat4    m_transform;
        float   m_fov;
        float   m_aspect;
        float   m_zNear;
        float   m_zFar;
        bool    m_dirty;

        friend class CameraController;
    };

    class CameraController
    {
    public:
        CameraController(Camera* pCamera = nullptr);
        void Update(const Window* pWindow);
        inline void SetCamera(Camera* pCamera) { m_pCamera = pCamera; }
        inline const Camera* GetCamera() const { return m_pCamera; }
        inline bool IsDirty() const { return m_dirty; }
    private:
        Camera* m_pCamera;
        bool m_dirty;
    };

} // namespace pbr
