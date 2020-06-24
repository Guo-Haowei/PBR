ADD_LIBRARY(pbr
    core/Application.cpp
    core/Camera.cpp
    core/Renderer.cpp
    core/Window.cpp
    Mesh.cpp
    Utility.cpp
    main.cpp
)

IF (OPENGL_RENDERER)
    ADD_SUBDIRECTORY(opengl)
    TARGET_LINK_LIBRARIES(pbr PRIVATE gl_renderer)
ENDIF ()

IF (DIRECT3D11_RENDERER)
    ADD_SUBDIRECTORY(direct3d)
    TARGET_LINK_LIBRARIES(pbr PRIVATE d3d11_renderer)
ENDIF ()

IF (METAL_RENDERER)
    ADD_SUBDIRECTORY(metal)
    TARGET_LINK_LIBRARIES(pbr PRIVATE mt_renderer)
ENDIF ()

IF (VULKAN_RENDERER)
    ADD_SUBDIRECTORY(vulkan)
    TARGET_LINK_LIBRARIES(pbr PRIVATE vk_renderer)
ENDIF ()

TARGET_INCLUDE_DIRECTORIES(pbr PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/external/stb
)

IF (NOT ${TARGET_PLATFORM} MATCHES "emscripten")
    TARGET_LINK_LIBRARIES(pbr PRIVATE glfw)
    TARGET_INCLUDE_DIRECTORIES(pbr PRIVATE
        ${PROJECT_SOURCE_DIR}/external/glfw/include
    )
ENDIF ()

TARGET_COMPILE_DEFINITIONS(pbr PRIVATE -DDATA_DIR="${PROJECT_SOURCE_DIR}/data/")

ADD_LIBRARY(pbr::pbr ALIAS pbr)
