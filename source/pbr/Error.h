#pragma once
#include <string>
#include <assert.h>
#include <ostream>

#if defined(_DEBUG)
#else
#endif

namespace pbr {
    using std::string;

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
