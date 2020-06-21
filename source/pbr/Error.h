#pragma once
#include <assert.h>
#include <stdexcept>

#if defined(_DEBUG)
#else
#endif

namespace pbr {
    using std::runtime_error;
} // namespace pbr
