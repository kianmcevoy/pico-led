#ifndef IDSP_LOOKUP_H
#define IDSP_LOOKUP_H

#include "idsp/functions.hpp"
#include "idsp/std_helpers.hpp"

#include <array>

namespace idsp
{

/** Class for generating, storing and using non-modifyable lookup tables. */
template<class T, size_t S>
class LookupTable
{
    static_assert(S > 0, "Cannot create lookup table of size 0.");

    static T _generator_wrapper(T v, T (*gen)(T))
    {
        return gen(v);
    }

    public:
        /** Generates the lookup table by calling the `generator` function with
         * arguments 0 to 1 inclusive, incrementing linearly.
         * This constuctor also allows for passing arbitrary data to the
         * generator function.
         */
        template<class A>
        LookupTable(T (*generator)(T, A*), A* data)
        {
            for (size_t i = 0; i < S; i++)
            {
                const T arg = static_cast<T>(i) / static_cast<T>(S - 1);
                const T value = generator(arg, data);
                this->_table[i] = value;
            }
        }

        /** Generates the lookup table by calling the `generator` function with
         * arguments 0 to 1 inclusive, incrementing linearly.
         */
        LookupTable(T (*generator)(T)):
        LookupTable(_generator_wrapper, generator)
        {}

        ~LookupTable() = default;

        /** Reads the lookup table given an index as separate integral and
         * fractional parts.
         */
        template<class I, class F,
        typename std::enable_if<std::is_integral<I>::value && std::is_floating_point<F>::value, bool>::type = true>
        T read(I index, F fraction) const
        {
            return idsp::interpolate_2(fraction, this->_table[index], this->_table[index + 1]);
        }
        /** Reads the lookup table given a normalised index, [0:1]. */
        template<class F,
        typename std::enable_if<std::is_floating_point<F>::value, bool>::type = true>
        T read(F index) const
        {
            const F scaled = index * static_cast<F>(S - 1);
            const size_t ind = static_cast<size_t>(scaled);
            const F frac = scaled - static_cast<F>(ind);
            return this->read(ind, frac);
        }

        /** Reads the lookup table given an index as separate integral and
         * fractional parts, with bounds limitation.
         */
        template<class I, class F,
        typename std::enable_if<std::is_integral<I>::value && std::is_floating_point<F>::value, bool>::type = true>
        T read_clamp(I index, F fraction) const
        {
            return idsp::interpolate_2(
                fraction,
                this->_table[idsp::clamp<I>(index, 0, S - 1)],
                this->_table[idsp::clamp<I>(index + 1, 0, S - 1)]
            );
        }
        /** Reads the lookup table given a normalised index, [0:1], with bounds
         * limitation.
         */
        template<class F,
        typename std::enable_if<std::is_floating_point<F>::value, bool>::type = true>
        T read_clamp(F index) const
        {
            index = idsp::clamp(index, F(0), F(1));
            return this->read(index);
        }

        /** Reads the lookup table given a normalised index, [0:1], and wraps
         * values greater > 1.
         */
        template<class F,
        typename std::enable_if<std::is_floating_point<F>::value, bool>::type = true>
        T read_wrap(F index) const
        {
            index = idsp::wrap(index);
            return this->read(index);
        }

        /** Element access. */
        IDSP_CONSTEXPR_SINCE_CXX14 Sample& operator[](size_t i)
            { return this->_table[i]; }
        constexpr const Sample& operator[](size_t i) const
            { return this->_table[i]; }

        /** @returns A const reference to the underlying container. */
        const std::array<T, S>& table() const
        {
            return this->_table;
        }

    private:
        std::array<T, S> _table;
};

} // namespace idsp

#endif
