#ifndef IDSP_STD_HELPERS_H
#define IDSP_STD_HELPERS_H

#ifndef __cplusplus
#   error "IDSP cannot be compiled as C; it is a C++ library"
#endif

#if __cplusplus >= 202302L
    #define IDSP_CXX_STANDARD 23
#elif __cplusplus >= 202002L
    #define IDSP_CXX_STANDARD 20
#elif __cplusplus >= 201703L
    #define IDSP_CXX_STANDARD 17
#elif __cplusplus >= 201402L
    #define IDSP_CXX_STANDARD 14
#elif __cplusplus >= 201103L
    #define IDSP_CXX_STANDARD 11
#else
    #define IDSP_CXX_STANDARD 1
#endif

#if IDSP_CXX_STANDARD >= 23
#define IDSP_CONSTEXPR_SINCE_CXX23 constexpr
#else
#define IDSP_CONSTEXPR_SINCE_CXX23
#endif

#if IDSP_CXX_STANDARD >= 20
#define IDSP_CONSTEXPR_SINCE_CXX20 constexpr
#else
#define IDSP_CONSTEXPR_SINCE_CXX20
#endif

#if IDSP_CXX_STANDARD >= 17
#define IDSP_CONSTEXPR_SINCE_CXX17 constexpr
#else
#define IDSP_CONSTEXPR_SINCE_CXX17
#endif

#if IDSP_CXX_STANDARD >= 14
#define IDSP_CONSTEXPR_SINCE_CXX14 constexpr
#else
#define IDSP_CONSTEXPR_SINCE_CXX14
#endif

#endif
