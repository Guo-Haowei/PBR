#pragma once
#include "base/Platform.h"
#include "base/Definitions.h"

#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
#   include <GLES3/gl32.h>
#   include <GLES2/gl2ext.h>
#else
#   include <glad/glad.h>
#endif

#include <GLFW/glfw3.h>

#define PBR_GL_VERSION (PBR_GL_VERSION_MAJOR * 100 + PBR_GL_VERSION_MINOR * 10)
