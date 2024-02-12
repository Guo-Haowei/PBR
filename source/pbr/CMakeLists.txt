ADD_LIBRARY(pbr
    core/Application.cpp
    core/Camera.cpp
    core/Renderer.cpp
    core/Window.cpp
    Mesh.cpp
    Utility.cpp
    main.cpp
)

IF (WIN32)
    ADD_EXECUTABLE(assimp_loader AssimpLoader.cpp)
    TARGET_INCLUDE_DIRECTORIES(assimp_loader PRIVATE ${PROJECT_SOURCE_DIR}/external/assimp/include)
    SET(lib_assimp_debug "${PROJECT_SOURCE_DIR}/external/assimp/build/lib/Debug/assimp-vc143-mtd.lib")
    SET(lib_irrxmld_debug "${PROJECT_SOURCE_DIR}/external/assimp/build/lib/Debug/IrrXMLd.lib")
    SET(lib_zlib_debug "${PROJECT_SOURCE_DIR}/external/assimp/build/lib/Debug/zlibstaticd.lib")
    TARGET_LINK_LIBRARIES(assimp_loader debug ${lib_assimp_debug} ${lib_irrxmld_debug} ${lib_zlib_debug})
ENDIF ()

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
