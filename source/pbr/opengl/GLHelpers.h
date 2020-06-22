#pragma once
#include "GLPrerequisites.h"
#include <string>

namespace pbr {
    using std::string;

    class GlslProgram
    {
    public:
        static GlslProgram create(GLuint vertHandle, GLuint fragHandle);
        static GLuint createShaderFromString(const string& source, GLenum type);

        void use();
    private:
        GLuint m_handle = 0;
    };

} // namespace pbr
