#include "base/Error.h"
#include "GLHelpers.h"
#include "Utility.h"

namespace pbr { namespace gl {

void GlslProgram::use() const
{
    glUseProgram(m_handle);
}

void GlslProgram::destroy()
{
    glDeleteProgram(m_handle);
    m_handle = 0;
}

GLint GlslProgram::getUniformLocation(const char* name) const
{
    GLint location = glGetUniformLocation(m_handle, name);
    if (location == INVALID_UNIFORM_LOCATION)
        cout << "[Warning] uniform \"" << name << "\" not found" << endl;
    return location;
}

void GlslProgram::setUniform(GLint location, const int& val) const
{
    glUniform1i(location, val);
}

void GlslProgram::setUniform(GLint location, const float& val) const
{
    glUniform1f(location, val);
}

void GlslProgram::setUniform(GLint location, const vec2& val) const
{
    glUniform2f(location, val.x, val.y);
}

void GlslProgram::setUniform(GLint location, const vec3& val) const
{
    glUniform3f(location, val.x, val.y, val.z);
}

void GlslProgram::setUniform(GLint location, const vec4& val) const
{
    glUniform4f(location, val.x, val.y, val.z, val.w);
}

void GlslProgram::setUniform(GLint location, const mat4& val) const
{
    glUniformMatrix4fv(location, 1, GL_FALSE, &val[0].x);
}

GLuint GlslProgram::createShaderFromString(const string& source, GLenum type)
{
    const int MAX_LOG_SIZE = 512;
    GLuint handle = glCreateShader(type);
    const char* sources[] = { source.c_str() };
    glShaderSource(handle, 1, sources, NULL);
    glCompileShader(handle);
    int success;
    char log[MAX_LOG_SIZE];
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(handle, MAX_LOG_SIZE, NULL, log);
        log[MAX_LOG_SIZE - 1] = '\0'; // prevent overflow
        string error("glsl: Failed to compile shader\n");
        error.append(log).pop_back(); // remove new line
        THROW_EXCEPTION(error);
    }

    return handle;
}

GlslProgram GlslProgram::create(GLuint vertHandle, GLuint fragHandle)
{
    GLuint handle = glCreateProgram();
    glAttachShader(handle, vertHandle);
    glAttachShader(handle, fragHandle);
    glLinkProgram(handle);

    const int MAX_LOG_SIZE = 512;
    int success;
    char log[MAX_LOG_SIZE];
    glGetProgramiv(handle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(handle, MAX_LOG_SIZE, NULL, log);
        log[MAX_LOG_SIZE - 1] = '\0'; // prevent overflow
        string error("glsl: Failed to link program\n");
        error.append(log).pop_back(); // remove new line
        THROW_EXCEPTION(error);
    }

    glDeleteShader(vertHandle);
    glDeleteShader(fragHandle);

    GlslProgram program;
    program.m_handle = handle;
    return program;
}

#if PBR_GL_VERSION >= 430 && defined(PBR_DEBUG)
void APIENTRY DebugCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}
#endif

} } // namespace pbr::gl
