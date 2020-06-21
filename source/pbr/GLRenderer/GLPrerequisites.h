#pragma once
#include "Platform.h"
#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
#   include <GLES3/gl32.h>
#   include <GLES2/gl2ext.h>
#else
#   include <glad/glad.h>
#endif
