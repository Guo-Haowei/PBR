CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
PROJECT(PBR)

SET(CMAKE_CXX_STANDARD 17)

# options
OPTION(BUILD_WITH_EMCMAKE "Build with emcmake" OFF)

# global variables
SET (OPENGL_RENDERER TRUE)
SET (DIRECT3D11_RENDERER FALSE)
SET (METAL_RENDERER FALSE)

SET (BUILD_GLFW TRUE)
SET (BUILD_GLAD TRUE)
SET (BUILD_GLM TRUE)

# configuration
IF (${BUILD_WITH_EMCMAKE})
    SET (TARGET_PLATFORM "emscripten")
    SET (BUILD_GLFW FALSE)
    SET (BUILD_GLAD FALSE)
    SET(CMAKE_EXECUTABLE_SUFFIX ".html")
    SET(EMSCRIPTEN_FLAGS "-s INITIAL_MEMORY=134217728 -s DISABLE_EXCEPTION_CATCHING=0 -s LEGACY_VM_SUPPORT=1 -s FULL_ES2=1 -s FULL_ES3=1 -s USE_WEBGL2=1 -s USE_GLFW=3 ")
    SET(EMSCRIPTEN_PRELOAD_FILES " --preload-file ${PROJECT_SOURCE_DIR}/data/preload/ --preload-file ${PROJECT_SOURCE_DIR}/data/models/cerberus/")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EMSCRIPTEN_FLAGS} ${EMSCRIPTEN_PRELOAD_FILES}")
ELSEIF (WIN32)
    SET (TARGET_PLATFORM "Windows")
    SET (DIRECT3D11_RENDERER TRUE)
ELSEIF (APPLE)
    SET (CMAKE_C_FLAGS "-x objective-c")
    SET (CMAKE_CXX_FLAGS "-x objective-c++")
    SET (TARGET_PLATFORM "macOS")
    SET (METAL_RENDERER TRUE)
ELSE ()
    MESSAGE (FATAL_ERROR "Unsupported platform")
ENDIF ()

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

MESSAGE ("************************************* Variables **********************************")
MESSAGE ("Platform:")
MESSAGE (STATUS "Target platform:               ${TARGET_PLATFORM}")
MESSAGE (STATUS "Compiler:                      ${CMAKE_CXX_COMPILER_ID}")
MESSAGE ("Renderer:")
MESSAGE (STATUS "OpenGL renderer:               ${OPENGL_RENDERER}")
MESSAGE (STATUS "Direct3D 11 renderer:          ${DIRECT3D11_RENDERER}")
MESSAGE (STATUS "Metal renderer:                ${METAL_RENDERER}")
MESSAGE ("Dependencies:")
MESSAGE (STATUS "Build GLFW:                    ${BUILD_GLFW}")
MESSAGE (STATUS "Build GLAD:                    ${BUILD_GLAD}")
MESSAGE (STATUS "Build GLM:                     ${BUILD_GLM}")
MESSAGE ("************************************* Dependencies *******************************")

IF (BUILD_GLAD)
    MESSAGE(STATUS "Configuring glfw...")
    SET(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    SET(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    SET(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    ADD_SUBDIRECTORY(external/glfw)
ENDIF ()

IF (BUILD_GLFW)
    MESSAGE(STATUS "Configuring glad...")
    ADD_SUBDIRECTORY(external/glad)
ENDIF ()

IF (BUILD_GLM)
    MESSAGE(STATUS "Configuring glm...")
    INCLUDE_DIRECTORIES(external/glm)
ENDIF ()

ADD_SUBDIRECTORY(source)

# IF (WIN32)
#     ADD_SUBDIRECTORY(tool)
# ENDIF ()
