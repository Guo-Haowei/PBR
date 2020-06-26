#pragma once
#include "Prerequisites.h"

namespace pbr {

    enum class DataType
    {
        UINT_8T,
        UINT_16T,
        UINT_32T,
        FLOAT_32T,
    };

    struct Buffer
    {
        void* pData;
        size_t sizeInByte;
    };

    struct Image
    {
        int width, height;
        int component;
        DataType dataType;
        Buffer buffer;
    };

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
        constexpr Extent2(U w, U h) : width(static_cast<T>(w)) , height(static_cast<T>(h)) { }
        constexpr Extent2() : Extent2(0, 0) { }

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
