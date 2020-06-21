#pragma once
#include "Platform.h"
#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
#else
#   include <glad/glad.h>
#endif
