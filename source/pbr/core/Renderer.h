#pragma once
#include "base/Definitions.h"
#include "Mesh.h"

namespace pbr {

class Window;
class Camera;

class Renderer
{
public:
    static constexpr int cubeMapRes             { 512 };
    static constexpr int irradianceMapRes       { 32 };
    static constexpr int prefilteredMaxMipLevel { 5 };
    static constexpr int prefilteredMapRes      { 512 };

    static Renderer* CreateRenderer(const Window* pWindow);
    virtual void Initialize() = 0;
    virtual void DumpGraphicsCardInfo() = 0;
    virtual void PrepareGpuResources() = 0;
    virtual void Render(const Camera& camera) = 0;
    virtual void Resize(const Extent2i& extent) = 0;
    virtual void Finalize() = 0;
    virtual ~Renderer() = default;
protected:
    Renderer(const Window* pWindow);
};

} // namespace pbr
