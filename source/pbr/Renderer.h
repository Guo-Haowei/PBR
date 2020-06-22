#pragma once
#include "Definitions.h"
#include "Mesh.h"

namespace pbr {

class Window;

class Renderer
{
public:
    static Renderer* CreateRenderer(const Window* pWindow);
    virtual void Initialize() = 0;
    virtual void DumpGraphicsCardInfo() = 0;
    virtual void PrepareGpuResources() = 0;
    virtual void Render() = 0;
    virtual void Resize(const Extent2i& extent) = 0;
    virtual void Finalize() = 0;
    virtual ~Renderer() = default;
protected:
    Renderer(const Window* pWindow);
protected:
    const Window* m_pWindow = nullptr;
};

} // namespace pbr
