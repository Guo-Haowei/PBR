#include "GLHelpers.h"
#include "Utility.h"
#include "base/Error.h"

namespace pbr {
namespace gl {

GLTexture CreateTexture(const Image& image, GLenum internalFormat) {
    GLenum imageFormat;
    switch (image.component) {
        case 4:
            imageFormat = GL_RGBA;
            break;
        case 3:
            imageFormat = GL_RGB;
            break;
        case 2:
            imageFormat = GL_RG;
            break;
        case 1:
            imageFormat = GL_RED;
            break;
        default:
            THROW_EXCEPTION("[texture] Unsupported image format, image has component " + std::to_string(image.component));
    }
    GLenum dataType;
    switch (image.dataType) {
        case DataType::FLOAT_32T:
            dataType = GL_FLOAT;
            break;
        case DataType::UINT_8T:
            dataType = GL_UNSIGNED_BYTE;
            break;
        default:
            THROW_EXCEPTION("[texture] Unsupported image format, image invalid data type");
    }
    GLTexture texture;
    texture.type = GL_TEXTURE_2D;
    glGenTextures(1, &texture.handle);
    glBindTexture(texture.type, texture.handle);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, image.width, image.height, 0, imageFormat, dataType, image.buffer.pData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    return texture;
}

GLTexture CreateEmptyCubeMap(int size, int mipmap) {
    GLTexture cubeTexture;
    cubeTexture.type = GL_TEXTURE_CUBE_MAP;
    glGenTextures(1, &cubeTexture.handle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture.handle);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                    mipmap > 0 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

    for (int i = 0; i < 6; ++i) {
#if TARGET_PLATFORM == PLATFORM_EMSCRIPTEN
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA32F, size, size, 0, GL_RGBA, GL_FLOAT, 0);
#else
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, size, size, 0, GL_RGB, GL_FLOAT, 0);
#endif
    }

    if (mipmap)
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    return cubeTexture;
}

void GlslProgram::use() const {
    glUseProgram(m_handle);
}

void GlslProgram::destroy() {
    glDeleteProgram(m_handle);
    m_handle = 0;
}

GLint GlslProgram::getUniformLocation(const char* name) const {
    GLint location = glGetUniformLocation(m_handle, name);
    // if (location == INVALID_UNIFORM_LOCATION)
    //     cout << "[Warning] uniform \"" << name << "\" not found" << endl;
    return location;
}

void GlslProgram::setUniform(GLint location, const int& val) const {
    glUniform1i(location, val);
}

void GlslProgram::setUniform(GLint location, const float& val) const {
    glUniform1f(location, val);
}

void GlslProgram::setUniform(GLint location, const vec2& val) const {
    glUniform2f(location, val.x, val.y);
}

void GlslProgram::setUniform(GLint location, const vec3& val) const {
    glUniform3f(location, val.x, val.y, val.z);
}

void GlslProgram::setUniform(GLint location, const vec4& val) const {
    glUniform4f(location, val.x, val.y, val.z, val.w);
}

void GlslProgram::setUniform(GLint location, const mat4& val) const {
    glUniformMatrix4fv(location, 1, GL_FALSE, &val[0].x);
}

GLuint GlslProgram::createShaderFromString(const string& source, GLenum type) {
    const int MAX_LOG_SIZE = 512;
    GLuint handle = glCreateShader(type);
    const char* sources[] = { source.c_str() };
    glShaderSource(handle, 1, sources, NULL);
    glCompileShader(handle);
    int success;
    char log[MAX_LOG_SIZE];
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(handle, MAX_LOG_SIZE, NULL, log);
        log[MAX_LOG_SIZE - 1] = '\0';  // prevent overflow
        string error("glsl: Failed to compile shader\n");
        error.append(log).pop_back();  // remove new line
        THROW_EXCEPTION(error);
    }

    return handle;
}

GlslProgram GlslProgram::create(GLuint vertHandle, GLuint fragHandle) {
    GLuint handle = glCreateProgram();
    glAttachShader(handle, vertHandle);
    glAttachShader(handle, fragHandle);
    glLinkProgram(handle);

    const int MAX_LOG_SIZE = 512;
    int success;
    char log[MAX_LOG_SIZE];
    glGetProgramiv(handle, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(handle, MAX_LOG_SIZE, NULL, log);
        log[MAX_LOG_SIZE - 1] = '\0';  // prevent overflow
        string error("glsl: Failed to link program\n");
        error.append(log).pop_back();  // remove new line
        THROW_EXCEPTION(error);
    }

    glDeleteShader(vertHandle);
    glDeleteShader(fragHandle);

    GlslProgram program;
    program.m_handle = handle;
    return program;
}

#if PBR_GL_VERSION >= 430 && defined(PBR_DEBUG)
void APIENTRY DebugCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* userParam) {
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;
    switch (source) {
        case GL_DEBUG_SOURCE_API:
            std::cout << "Source: API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            std::cout << "Source: Window System";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            std::cout << "Source: Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            std::cout << "Source: Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            std::cout << "Source: Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            std::cout << "Source: Other";
            break;
    }
    std::cout << std::endl;
    switch (type) {
        case GL_DEBUG_TYPE_ERROR:
            std::cout << "Type: Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            std::cout << "Type: Deprecated Behaviour";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            std::cout << "Type: Undefined Behaviour";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            std::cout << "Type: Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            std::cout << "Type: Performance";
            break;
        case GL_DEBUG_TYPE_MARKER:
            std::cout << "Type: Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            std::cout << "Type: Push Group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            std::cout << "Type: Pop Group";
            break;
        case GL_DEBUG_TYPE_OTHER:
            std::cout << "Type: Other";
            break;
    }
    std::cout << std::endl;
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH:
            std::cout << "Severity: high";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            std::cout << "Severity: medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            std::cout << "Severity: low";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            std::cout << "Severity: notification";
            break;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}
#endif

}  // namespace gl
}  // namespace pbr
