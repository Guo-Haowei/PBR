#pragma once
#include "Prerequisites.h"

namespace pbr {

    enum class RenderApi { UNKNOWN, OPENGL, DIRECT3D11, VULKAN, METAL };

    static const string& RenderApiToString(RenderApi api)
    {
        static const string sTable[static_cast<int>(RenderApi::METAL) + 1] = {
            "Unknown", "OpenGL", "Direct3d 11", "Vulkan", "Metal"
        };

        return sTable[static_cast<int>(api)];
    }

    template <typename T> struct Extent2
    {
        T width, height;
        template <typename U>
        Extent2(U w, U h) : width(static_cast<T>(w)) , height(static_cast<T>(h)) { }
        Extent2() : Extent2(0, 0) { }

        friend ostream& operator<<(ostream& os, const Extent2<T>& e)
        {
            os << "(width: " << e.width << ", " << e.height << ")";
            return os;
        }
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
