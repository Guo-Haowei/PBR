ADD_LIBRARY(mt_renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/MtRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/MtRendererImpl.mm
)

TARGET_INCLUDE_DIRECTORIES(mt_renderer PRIVATE
    ${PROJECT_SOURCE_DIR}/source/pbr
    ${PROJECT_SOURCE_DIR}/external/glfw/include
)

TARGET_LINK_LIBRARIES(mt_renderer PRIVATE
    "-framework Metal"
    "-framework Cocoa"
    "-framework QuartzCore"
)

TARGET_COMPILE_DEFINITIONS(mt_renderer PRIVATE -DDATA_DIR=@"${PROJECT_SOURCE_DIR}/data/")
