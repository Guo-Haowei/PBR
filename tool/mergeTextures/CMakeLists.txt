ADD_EXECUTABLE(mergeTextures main.cpp)

TARGET_INCLUDE_DIRECTORIES(mergeTextures PRIVATE
    ${PROJECT_SOURCE_DIR}/external/stb/
)

