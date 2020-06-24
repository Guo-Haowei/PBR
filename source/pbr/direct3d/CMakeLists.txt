ADD_LIBRARY(d3d11_renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/D3d11Renderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/D3d11Helpers.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/D3d11RendererImpl.cpp
)

TARGET_INCLUDE_DIRECTORIES(d3d11_renderer PRIVATE
    ${PROJECT_SOURCE_DIR}/source/pbr
    ${PROJECT_SOURCE_DIR}/external/glfw/include
)

TARGET_LINK_LIBRARIES(d3d11_renderer PRIVATE
    d3d11.lib
    d3dcompiler.lib
)

TARGET_COMPILE_DEFINITIONS(d3d11_renderer PRIVATE -DDATA_DIR="${PROJECT_SOURCE_DIR}/data/")
