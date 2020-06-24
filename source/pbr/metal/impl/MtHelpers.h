#import "MtPrerequisites.h"
#import "base/Error.h"

namespace pbr { namespace mt {

    class NSException : public Exception
    {
    public:
        NSException(int line, const char* file, const char* desc)
            : Exception(line, file, desc)
        {
        }

        NSException(int line, const char* file, const string& desc)
            : Exception(line, file, desc)
        {
        }

        virtual ostream& dump(ostream& os) const
        {
            os << "[Error] Metal: " << m_desc << ".\n\ton line " << m_line << ", in file [" << m_file << "]";
            return os;
        }
    };

} } // namespace pbr::mt

#define MT_THROW_NS_STRING(DETAIL) \
{ \
    const string detail = string([DETAIL UTF8String]); \
    throw pbr::mt::NSException(__LINE__, __FILE__, detail); \
}
