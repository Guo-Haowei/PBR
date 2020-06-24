ADD_LIBRARY(mtRenderer
    ${CMAKE_CURRENT_SOURCE_DIR}/MtRenderer.mm
)

TARGET_INCLUDE_DIRECTORIES(mtRenderer PRIVATE
    ${PROJECT_SOURCE_DIR}/source/pbr
    ${PROJECT_SOURCE_DIR}/external/glfw/include
    ${PROJECT_SOURCE_DIR}/external/glad/include
)

TARGET_LINK_LIBRARIES(mtRenderer PRIVATE "-framework Metal"
                                         "-framework Cocoa"
                                         "-framework QuartzCore")
