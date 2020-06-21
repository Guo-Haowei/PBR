#pragma once
#define PLATFORM_WINDOWS 0
#define PLATFORM_MACOS 1
#define PLATFORM_EMSCRIPTEN 2
#if defined(__EMSCRIPTEN__)
#   define TARGET_PLATFORM PLATFORM_EMSCRIPTEN
#elif defined(_WIN32)
#   define TARGET_PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
#   define TARGET_PLATFORM PLATFORM_MACOS
#else
#   error "Unsupported platform"
#endif

// include opengl
#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
#   include <emscripten.h>
#   include <GLES3/gl32.h>
#   include <GLES2/gl2ext.h>
#else
#   include <glad/glad.h>
#endif

// opengl version
#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
#   define PBR_GL_VERSION_MAJOR 3
#   define PBR_GL_VERSION_MINOR 0
#elif TARGET_PLATFORM == PLATFORM_WINDOWS
#   define PBR_GL_VERSION_MAJOR 4
#   define PBR_GL_VERSION_MINOR 6
#elif TARGET_PLATFORM == PLATFORM_MACOS
#   define PBR_GL_VERSION_MAJOR 4
#   define PBR_GL_VERSION_MINOR 1
#endif

#define PBR_GL_VERSION (PBR_GL_VERSION_MAJOR * 100 + PBR_GL_VERSION_MINOR * 10)

// glfw
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
