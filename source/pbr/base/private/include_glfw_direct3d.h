// include opengl
#include "../Platform.h"

#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
#   include <GLFW/glfw3.h>
#elif TARGET_PLATFORM == PLATFORM_WINDOWS
#   define GLFW_INCLUDE_VULKAN
#   define GLFW_EXPOSE_NATIVE_WIN32
#   include <GLFW/glfw3.h>
#   include <GLFW/glfw3native.h>
#elif TARGET_PLATFORM == PLATFORM_MACOS
#   define GLFW_INCLUDE_VULKAN
#   define GLFW_EXPOSE_NATIVE_COCOA
#   include <GLFW/glfw3.h>
#   include <GLFW/glfw3native.h>
#endif

