ADD_LIBRARY(pbr
    Application.cpp
    Renderer.cpp
    Window.cpp
    main.cpp
)

IF (OPENGL_RENDERER)
    TARGET_SOURCES(pbr PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/GLRenderer/GLRenderer.cpp)
ENDIF ()

IF (DIRECT3D11_RENDERER)
    TARGET_SOURCES(pbr PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/D3DRenderer/D3D11Renderer.cpp)
ENDIF ()

TARGET_INCLUDE_DIRECTORIES(pbr PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

IF (NOT ${TARGET_PLATFORM} MATCHES "emscripten")
    TARGET_LINK_LIBRARIES(pbr PRIVATE glad glfw)
    TARGET_INCLUDE_DIRECTORIES(pbr PRIVATE
        ${PROJECT_SOURCE_DIR}/external/glad/include
        ${PROJECT_SOURCE_DIR}/external/glfw/include
    )
ENDIF ()

ADD_LIBRARY(pbr::pbr ALIAS pbr)
