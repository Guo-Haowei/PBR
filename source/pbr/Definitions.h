#pragma once
#include <string>
#include <memory>

namespace pbr {
    using std::string;
    using std::unique_ptr;
    using std::shared_ptr;

    enum class RenderApi { UNKNOWN, OPENGL, DIRECT3D11, VULKAN, METAL };

    template <typename T> struct Extent2
    {
        T width, height;
        template <typename U>
        Extent2(U w, U h) : width(static_cast<T>(w)) , height(static_cast<T>(h)) { }
        Extent2() : Extent2(0, 0) { }
    };

    typedef Extent2<int> Extent2i;

    struct WindowCreateInfo
    {
        Extent2i        extent;
        float           windowScale;
        bool            resizable;
        RenderApi       renderApi;
        // msaa
        // vsync

        WindowCreateInfo(RenderApi renderApi, float windowScale, const Extent2i& extent, bool resizable)
            : extent(extent)
            , windowScale(windowScale)
            , resizable(resizable)
            , renderApi(renderApi)
        {
        }
    };

} // namespace pbr
