ADD_EXECUTABLE(brdfLutGenerator
    main.cpp
)

TARGET_LINK_LIBRARIES(brdfLutGenerator
    glad
    glfw
)

TARGET_INCLUDE_DIRECTORIES(brdfLutGenerator PRIVATE
    ${PROJECT_SOURCE_DIR}/external/glfw/include
    ${PROJECT_SOURCE_DIR}/external/glad/include
    ${PROJECT_SOURCE_DIR}/external/stb/
)

TARGET_COMPILE_DEFINITIONS(brdfLutGenerator PRIVATE
    -DSHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/"
)
