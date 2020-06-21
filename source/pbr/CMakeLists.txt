ADD_LIBRARY(pbr
    Application.cpp
    Window.cpp
    main.cpp
)

IF (NOT ${TARGET_PLATFORM} MATCHES "emscripten")
    TARGET_LINK_LIBRARIES(pbr PRIVATE glfw)
    TARGET_INCLUDE_DIRECTORIES(pbr PRIVATE ${PROJECT_SOURCE_DIR}/glfw/include)
ENDIF ()

ADD_LIBRARY(pbr::pbr ALIAS pbr)
