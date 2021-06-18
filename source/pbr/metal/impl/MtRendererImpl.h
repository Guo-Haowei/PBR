#pragma once
#include "base/Definitions.h"
#include "core/Camera.h"

namespace pbr {
namespace mt {

class MtRendererImpl {
   public:
    MtRendererImpl(const Window* pWindow);
    void Initialize();
    void DumpGraphicsCardInfo();
    void PrepareGpuResources();
    void Render(const Camera& camera);
    void Resize(const Extent2i& extent);
    void Finalize();

   private:
    const Window* m_pWindow;
};

}  // namespace mt
}  // namespace pbr
