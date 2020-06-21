#pragma once

namespace pbr {

class Window;

class Renderer
{
public:
    static Renderer* CreateRenderer(const Window* pWindow);
    virtual void Initialize() = 0;
    virtual void Render() = 0;
    virtual void Finalize() = 0;
    virtual ~Renderer() = default;
protected:
    Renderer(const Window* pWindow);
protected:
    const Window* m_pWindow = nullptr;
};

} // namespace pbr
