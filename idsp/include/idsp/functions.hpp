#ifndef IDSP_FUNCTIONS_H
#define IDSP_FUNCTIONS_H

#include "idsp/buffer_interface.hpp"
#include "idsp/std_helpers.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <vector>



namespace idsp
{
    /** Scales `x` from the range (`xMin`, `xMax`) to range (`yMin`, `yMax`). */
    template<typename T>
    constexpr T rescale(T x, T xMin, T xMax, T yMin, T yMax) {
        return yMin + (((x - xMin) * (yMax - yMin)) / (xMax - xMin));
    }

    /** Scales `x` from the range (`xMin`, `xMax`) to range (`yMin`, `yMax`) with a specified centre point. */
    template<typename T>
    constexpr T rescaleWithCentre(T x, T xMin, T xMax, T yMin, T yMax, T yCentre) {
        auto xCentre = xMin + (xMax - xMin) / 2;
        if (x <= xCentre) {
            return yMin + (((x - xMin) * (yCentre - yMin)) / (xCentre - xMin));
        } else {
            return yCentre + (((x - xCentre) * (yMax - yCentre)) / (xMax - xCentre));
        }
    }



    /** Signum function. */
    template<typename T>
    constexpr int sgn(T v) {
        return (v > T(0)) - (v < T(0));
    }

    /** @return the minimum of `a` and `b`. */
    template<typename T>
    constexpr T min(T a, T b) {
        return a < b ? a : b;
    }

    /** @return the maximum of `a` and `b`. */
    template<typename T>
    constexpr T max(T a, T b) {
        return a > b ? a : b;
    }

    /** Limits `x` between to `lo` and `hi`. */
    template<typename T>
    constexpr T clamp(T x, T lo, T hi) {
        return idsp::max(idsp::min(x, hi), lo);
    }

    /** Padé approximation of the hyperbolic tangent function. */
    template<typename T>
    constexpr T tanh_fast(T x) {
        #if IDSP_CXX_STANDARD > 11
            // constexpr T pi = M_PI;
            const T v {x * ((T(9)*pi) + (x * x)) / ((T(9)*pi) + ((T(3)*pi) * (x * x)))};
            return idsp::clamp(v, T(-1), T(1));
        #else
            // Hacky re-write to be C++11 compatible
            return clamp(x * ((T(9)*M_PI) + (x * x)) / ((T(9)*M_PI) + ((T(3)*M_PI) * (x * x))), T(-1), T(1));
        #endif
    }

    /** @returns `true` if @a lo <= @a x <= @a hi. */
    template<typename T>
    constexpr bool is_between(T x, T lo, T hi) {
        return (lo <= x) && (x <= hi);
    }

    /** @returns `true` if @a x is between @a a and @a b.
     * Similar to @ref is_between, but the bounds can be either way around. */
    template<typename T>
    constexpr bool is_between_safe(T x, T a, T b) {
        return idsp::is_between(x, min(a, b), max(a, b));
    }

    template<typename T,
    typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, bool>::type = true>
    constexpr T wrap_safe(T x, T lo, T hi) {
        #if IDSP_CXX_STANDARD > 11
            const auto r {hi - lo};
            if (x < lo)
                x += r * ((lo - x) / r + 1);
            return lo + (x - lo) % r;
        #else
            // Hacky re-write to be C++11 compatible - UNTESTED
            return lo + ((x >= lo ? x : (x + (hi - lo) * ((lo - x) / (hi - lo) + 1))) - lo) % (hi - lo);
        #endif
    }

    template<typename T,
    typename std::enable_if<std::is_integral<T>::value, bool>::type = true>
    constexpr T wrap(T x, T lo, T hi) {
        #if IDSP_CXX_STANDARD > 11
            const auto r {hi - lo};
            return x >= lo ? (x < hi ? x : x - r) : x + r;
        #else
            return x >= lo ? (x < hi ? x : x - (hi - lo)) : x + (hi - lo);
        #endif
    }

    /** Wraps a value to between 0 and 1. */
    template<typename T,
    typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
    constexpr T wrap(T x){
        return x - std::floor(x);
    }

    // Interpolation

    /** 2-point linear interpolation. */
    template<typename T, typename V,
    typename std::enable_if<std::is_convertible<V, T>::value, bool>::type = true>
    constexpr T interpolate_2(V frac, T n, T n1) {
        return n + ((n1 - n) * frac);
    }

    /** 4-point interpolation. */
    template<typename T, typename V,
    typename std::enable_if<std::is_convertible<V, T>::value, bool>::type = true>
    constexpr T interpolate_4(V frac, T a, T b, T c, T d) {
        return b + frac * ((c - b) - T(0.1666667) * (T(1) - frac) * ((d - a - T(3)*(c - b)) * frac + (d + T(2)*a - T(3)*b)));
    }

    /** Interpolates an idsp::SampleBuffer using 4-point interpolation.
     * @param buf idsp::SampleBuffer to interpolate.
     * @param index index of the 'current' sample.
     * @param frac fractional value to intepolate by.
     */
    template<typename V>
    constexpr Sample interpolate_4(const idsp::BufferInterface& buf, long int index, V frac) {
        return idsp::interpolate_4(frac, buf[index-1], buf[index], buf[index+1], buf[index+2]);
    }

    /** Interpolates an idsp::SampleBuffer using 4-point interpolation, wrapping
     * the read indices around the buffer's bounds once.
     * @param buf idsp::SampleBuffer to interpolate.
     * @param index index of the 'current' sample.
     * @param frac fractional value to intepolate by.
     */
    template<typename V>
    constexpr Sample interpolate_4_wrap(const idsp::BufferInterface& buf, long int index, V frac) {
        return idsp::interpolate_4(frac,
            buf[idsp::wrap<long int>(index - 1, 0, buf.size())],
            buf[idsp::wrap<long int>(index, 0, buf.size())],
            buf[idsp::wrap<long int>(index + 1, 0, buf.size())],
            buf[idsp::wrap<long int>(index + 2, 0, buf.size())]
        );
    }

    /** Interpolates an idsp::SampleBuffer using 4-point interpolation, wrapping
     * the read indices around the buffer's bounds infinitely.
     * @param buf idsp::SampleBuffer to interpolate.
     * @param index index of the 'current' sample.
     * @param frac fractional value to intepolate by.
     */
    template<typename V>
    constexpr Sample interpolate_4_safe(const idsp::BufferInterface& buf, long int index, V frac) {
        return idsp::interpolate_4(frac,
            buf[idsp::wrap_safe<long int>(index - 1, 0, buf.size())],
            buf[idsp::wrap_safe<long int>(index, 0, buf.size())],
            buf[idsp::wrap_safe<long int>(index + 1, 0, buf.size())],
            buf[idsp::wrap_safe<long int>(index + 2, 0, buf.size())]
        );
    }

    /** Interpolates an idsp::SampleBuffer using 4-point interpolation, clamping
     * the read indices to the buffer's bounds.
     * @param buf idsp::SampleBuffer to interpolate.
     * @param index index of the 'current' sample.
     * @param frac fractional value to intepolate by.
     */
    template<typename V>
    constexpr Sample interpolate_4_clamp(const idsp::BufferInterface& buf, long int index, V frac) {
        return idsp::interpolate_4(frac,
            buf[idsp::clamp<long int>(index - 1, 0, buf.size())],
            buf[idsp::clamp<long int>(index, 0, buf.size())],
            buf[idsp::clamp<long int>(index + 1, 0, buf.size())],
            buf[idsp::clamp<long int>(index + 2, 0, buf.size())]
        );
    }



    /** Returns $x^n$. */
    template<typename T, typename P,
    typename std::enable_if<std::is_unsigned<P>::value, bool>::type = true>
    constexpr T power(T x, P n) {
        T rv { 1 };

        for (P i { 0 }; i < n; i++)
            rv *= x;

        return rv;
    }

    /** Returns $x!$. */
    template<typename T,
    typename std::enable_if<std::is_integral<T>::value, bool>::type = true>
    constexpr T factorial(T x) {
        T rv = 1;
        for (typename std::make_signed<T>::type i = x; i > 0; i--)
            rv *= i;
        return rv;
    }

    /** Sine over argument function. */
    template<typename T,
    typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
    T sa(T x)
    {
        if (x == T(0))
            return T(1);
        x *= idsp::pi;
        return std::sin(x) / x;
    }

    /** 9-th order Taylor approximation of sin(x).
     * Only accurate for -pi < x < pi.
     */
    template<typename T>
    constexpr T sin_fast(T x) {
        return (
            x
            - (idsp::power(x, 3u) / idsp::factorial(3u))
            + (idsp::power(x, 5u) / idsp::factorial(5u))
            - (idsp::power(x, 7u) / idsp::factorial(7u))
            + (idsp::power(x, 9u) / idsp::factorial(9u))
            - (idsp::power(x, 11u) / idsp::factorial(11u))
        );
    }

    /** 9-th order Taylor approximation of sin(x).
     * Slower than @ref sin_fast but range-safe.
     */
    template<typename T>
    constexpr T sin_fast_safe(T x) {
        // constexpr T pi {M_PI};
        // constexpr T twopi {T(2) * pi};

        if ((x > twopi) or (x < T(0))) {
            x = std::fmod(x, twopi);
        }

        if (x > pi) {
            x -= twopi;
        }

        return idsp::sin_fast(x);
    }

    /** Returns a cosine window. */
    template<typename T,
    typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
    constexpr T cos_window(T phase){
        return T(0.5) - T(0.5) * std::cos(T(2) * T(M_PI) * phase);
    }

    template<typename T,
    typename std::enable_if<std::is_floating_point<T>::value, bool>::type = true>
    T blackman_harris_window(T phase)
    {
        return
            + T(0.35875)
            - T(0.48829) * std::cos(2 * idsp::pi * phase)
            + T(0.14128) * std::cos(4 * idsp::pi * phase)
            - T(0.01168) * std::cos(6 * idsp::pi * phase);
    }

    template<class T>
    constexpr void scale(idsp::BufferInterface& buffer, T scalar)
    {
        for (auto& x : buffer)
            x *= scalar;
    }

    template<typename T,
    typename std::enable_if<std::is_signed<T>::value, bool>::type = true>
    constexpr void mid_side(const T in_left, const T in_right,  T& out_mid, T& out_side)
    {
        out_mid = (in_left + in_right) / 2;
        out_side = (in_left - in_right) / 2;
    }

    template<typename T,
    typename std::enable_if<std::is_signed<T>::value, bool>::type = true>
    constexpr void side_mid(const T in_mid, const T in_side,  T& out_left, T& out_right)
    {
        out_left = in_mid + in_side;
        out_right = in_mid - in_side;
    }

}



#endif
