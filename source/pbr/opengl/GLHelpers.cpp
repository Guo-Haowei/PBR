#include "base/Error.h"
#include "GLHelpers.h"
#include "Utility.h"

namespace pbr {

void GlslProgram::use()
{
    glUseProgram(m_handle);
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

} // namespace pbr

