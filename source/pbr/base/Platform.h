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

#define HLSL_DIR DATA_DIR "shaders/hlsl/"
