#pragma once
#include "GLPrerequisites.h"

namespace pbr { namespace gl {

    struct PerDrawData
    {
        uint32_t vao = 0;
        uint32_t vbo = 0;
        uint32_t ebo = 0;
        uint32_t indexCount = 0;
    };

    class GlslProgram
    {
    public:
        enum : GLint { INVALID_UNIFORM_LOCATION = -1 };
    public:
        static GlslProgram create(GLuint vertHandle, GLuint fragHandle);
        static GLuint createShaderFromString(const string& source, GLenum type);

        void use() const;
        void setUniform(GLint location, const int& val) const;
        void setUniform(GLint location, const float& val) const;
        void setUniform(GLint location, const vec2& val) const;
        void setUniform(GLint location, const vec3& val) const;
        void setUniform(GLint location, const vec4& val) const;
        void setUniform(GLint location, const mat4& val) const;
        GLint getUniformLocation(const char* name) const;

        template<typename T> void setUniform(const char* name, const T& val)
        {
            GLint location = getUniformLocation(name);
            if (location == INVALID_UNIFORM_LOCATION)
                return;

            setUniform(location, val);
        }
        template<typename T> void setUniform(const string& name, const T& val)
        {
            GLint location = getUniformLocation(name.c_str());
            if (location == INVALID_UNIFORM_LOCATION)
                return;

            setUniform(location, val);
        }

        void destroy();
    private:
        GLuint m_handle = 0;
    };

#if PBR_GL_VERSION >= 430 && defined(PBR_DEBUG)
    extern void APIENTRY DebugCallback(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam);
#endif

} } // namespace pbr::gl
