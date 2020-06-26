#include "base/Prerequisites.h"
#include "base/Definitions.h"

namespace pbr {

    class Window;

    class Camera
    {
    public:
        Camera(float fov = QuarterPi, float aspect = 1.0f, float zNear = 0.1f, float zFar = 100.0f);
        inline const vec4& GetViewPos() const { return m_transform[3]; }
        static mat4 ViewMatrix(const mat4& transform);
        mat4 ViewMatrix() const;
        mat4 ProjectionMatrixD3d() const;
        mat4 ProjectionMatrixGl() const;
        inline void SetTransformation(const mat4& transform) { m_transform = transform; m_dirty = true; }
        inline void SetAspect(float aspect) { m_aspect = aspect; m_dirty = true; }
        inline void SetFov(float fov) { m_fov = fov; m_dirty = true; }
        inline bool IsDirty() const { return m_dirty; }
        inline void MarkDirty() { m_dirty = true; }
    protected:
        mat4    m_transform;
        float   m_fov;
        float   m_aspect;
        float   m_zNear;
        float   m_zFar;
        bool    m_dirty;

        friend class CameraController;
    };

    class CubeCamera : public Camera
    {
    public:
        using Camera::Camera;
        void ViewMatricesGl(array<mat4, 6>& inMatrices) const;
        void ViewMatricesD3d(array<mat4, 6>& inMatrices) const;
    };

    class CameraController
    {
    public:
        CameraController(Camera* pCamera = nullptr);
        void Update(const Window* pWindow);
        inline void SetCamera(Camera* pCamera) { m_pCamera = pCamera; }
        inline const Camera* GetCamera() const { return m_pCamera; }
        inline bool IsDirty() const { return m_dirty; }
        static bool virtualPoint(const Extent2i& extent, const vec2& cursorPos, vec3& point);
    private:
        Camera* m_pCamera;
        bool m_dirty;
    };

} // namespace pbr
