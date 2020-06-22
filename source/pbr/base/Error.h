#pragma once
#include "Prerequisites.h"
#include <assert.h>

namespace pbr {

    class Exception
    {
    public:
        Exception(int line, const char* file, const char* desc)
            : m_line(line), m_file(file), m_desc(desc)
        {
        }

        Exception(int line, const char* file, const string& desc)
            : m_line(line), m_file(file), m_desc(desc)
        {
        }

        virtual ostream& dump(ostream& os) const
        {
            os << "[Error] " << m_desc << ".\n\ton line " << m_line << ", in file [" << m_file << "]";
            return os;
        }
    protected:
        int         m_line;
        string      m_file;
        string      m_desc;
    };

    static ostream& operator<<(ostream& os, const Exception& e)
    {
        e.dump(os);
        return os;
    }
} // namespace pbr

#define THROW_EXCEPTION(DESC) throw pbr::Exception(__LINE__, __FILE__, DESC)

#ifdef PBR_VERBOSE
#define SHADER_COMPILING_START_INFO(shader) \
    std::cout << "--------------------------------------------\n"; \
    std::cout << "[Log] compiling shader " << shader << std::endl;

#define SHADER_COMPILING_END_INFO(shader) \
    std::cout << "[Log] shader " << shader << " compiled successfully\n"; \
    std::cout << "--------------------------------------------" << std::endl;
#else
#define SHADER_COMPILING_START_INFO(shader)
#define SHADER_COMPILING_END_INFO(shader)
#endif
