#pragma once
#include "Renderer.h"

namespace pbr {

class GLRenderer : public Renderer
{
public:
    GLRenderer(const Window* pWindow);
    virtual void Initialize() override;
    virtual void Render() override;
    virtual void Finalize() override;
private:
};

} // namespace pbr
