ADD_LIBRARY(mt_renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/MtRenderer.mm
)

TARGET_INCLUDE_DIRECTORIES(mt_renderer PRIVATE
    ${PROJECT_SOURCE_DIR}/source/pbr
    ${PROJECT_SOURCE_DIR}/external/glfw/include
    ${PROJECT_SOURCE_DIR}/external/glad/include
)

TARGET_LINK_LIBRARIES(mt_renderer PRIVATE "-framework Metal"
                                         "-framework Cocoa"
                                         "-framework QuartzCore")
