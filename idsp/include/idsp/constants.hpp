#ifndef IDSP_CONSTANTS
#define IDSP_CONSTANTS

#include "math.h"
#include <array>

namespace idsp
{

    #ifndef Sample
        #warning "IDSP buffer.hpp: `Sample` not defined, defaulting to `float`"
        #define Sample float
    #endif

    static constexpr float pi {static_cast<float>(M_PI)};

    static constexpr float twopi {2.f * pi};

}//namespace idsp

#endif
