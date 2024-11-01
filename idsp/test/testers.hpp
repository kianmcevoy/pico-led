#ifndef IDSP_TEST_HELPERS_H
#define IDSP_TEST_HELPERS_H

#include "idsp/functions.hpp"

#include <array>
#include <functional>
#include <iostream>
#include <string>

namespace idsp
{

namespace _help
{
    template<typename T>
    inline std::string string_ne(T a, T b)
    {
        return std::to_string(a) + " != " + std::to_string(b);
    }
} // namespace _help

static inline void test(bool condition, const std::string& message)
{
    if (!condition)
    {
        std::cerr << "Test failed: " + message << std::endl;
        exit(1);
    }
}

static inline void test(std::function<bool()> condition, const std::string& message)
{
    test(condition(), message);
}

template<typename T,
typename std::enable_if<std::is_integral<T>::value, bool>::type = true>
static inline void test_eq(T a, T b, const std::string& message)
{
    test(a == b, _help::string_ne(a, b) + "; " + message);
}

template<typename T,
typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
static inline void test_eq(T a, T b, const std::string& message, T margin = 1e-6)
{
    test(std::abs(a - b) < margin, _help::string_ne(a, b) + "; " + message);
}

} // namespace idsp

#endif
