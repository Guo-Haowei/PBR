ADD_LIBRARY(pbr
    Application.cpp
    Renderer.cpp
    Utility.cpp
    Window.cpp
    main.cpp
)

IF (OPENGL_RENDERER)
    TARGET_SOURCES(pbr PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/GLRenderer/GLHelpers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/GLRenderer/GLRenderer.cpp
    )
ENDIF ()

IF (DIRECT3D11_RENDERER)
    TARGET_SOURCES(pbr PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/D3dRenderer/D3d11Helpers.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/D3dRenderer/D3d11Renderer.cpp
    )
ENDIF ()

IF (VULKAN_RENDERER)
    MESSAGE ("************************************* Vulkan *************************************")
    FIND_PACKAGE(Vulkan REQUIRED FATAL_ERROR)
    TARGET_SOURCES(pbr PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/VkRenderer/VkRenderer.cpp)
    TARGET_INCLUDE_DIRECTORIES(pbr PRIVATE ${Vulkan_INCLUDE_DIRS})
    TARGET_LINK_LIBRARIES(pbr PRIVATE ${Vulkan_LIBRARIES})
    MESSAGE (STATUS "Vulkan_INCLUDE_DIRS:           ${Vulkan_INCLUDE_DIRS}")
    MESSAGE (STATUS "Vulkan_LIBRARIES:              ${Vulkan_LIBRARIES}")
    MESSAGE ("**********************************************************************************")
ENDIF ()

TARGET_INCLUDE_DIRECTORIES(pbr PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

IF (NOT ${TARGET_PLATFORM} MATCHES "emscripten")
    TARGET_LINK_LIBRARIES(pbr PRIVATE glad glfw)
    TARGET_INCLUDE_DIRECTORIES(pbr PRIVATE
        ${PROJECT_SOURCE_DIR}/external/glad/include
        ${PROJECT_SOURCE_DIR}/external/glfw/include
    )
ENDIF ()

TARGET_COMPILE_DEFINITIONS(pbr PUBLIC -DDATA_DIR="${PROJECT_SOURCE_DIR}/data/")

IF (${BUILD_DEBUG})
    TARGET_COMPILE_DEFINITIONS(pbr PUBLIC -DPBR_DEBUG -DPBR_VERBOSE)
ENDIF ()

ADD_LIBRARY(pbr::pbr ALIAS pbr)
