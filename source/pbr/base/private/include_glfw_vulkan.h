// include opengl
#include "../Platform.h"

#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
#   error "Vulkan not supported"
#endif

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
