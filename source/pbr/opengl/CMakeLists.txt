ADD_LIBRARY(gl_renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/GLRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/GLHelpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/GLRendererImpl.cpp
)

TARGET_INCLUDE_DIRECTORIES(gl_renderer PRIVATE
    ${PROJECT_SOURCE_DIR}/source/pbr
)

IF (NOT ${TARGET_PLATFORM} MATCHES "emscripten")
    TARGET_LINK_LIBRARIES(gl_renderer PRIVATE glad glfw)
    TARGET_INCLUDE_DIRECTORIES(gl_renderer PRIVATE
        ${PROJECT_SOURCE_DIR}/external/glfw/include
        ${PROJECT_SOURCE_DIR}/external/glad/include
    )
ENDIF ()

TARGET_COMPILE_DEFINITIONS(gl_renderer PRIVATE -DDATA_DIR="${PROJECT_SOURCE_DIR}/data/")
