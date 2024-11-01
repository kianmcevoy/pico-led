#ifndef IDSP_CONTROLS_H
#define IDSP_CONTROLS_H

#include "idsp/functions.hpp"
#include "idsp/std_helpers.hpp"

#include <cstdint>
#include <tuple>

namespace idsp
{

/** Binary state processor class. */
class Flag
{
    public:
        /** Enumeration of processing behaviours. */
        enum class Behaviour : uint8_t
        {
            /** Momentary behaviour.
             * The output is only high while the input is high.
             */
            Momentary,
            /** Latching behaviour.
             * The output changes state when the input flips from low to high.
             */
            Latching,
        };

        Flag(Flag::Behaviour behaviour = Flag::Behaviour::Momentary):
        behaviour{behaviour},
        state{false},
        prev_state{false},
        prev_input{false}
        {}

        ~Flag() = default;

        /** Processes a button's state. */
        void process(bool input)
        {
            switch (this->behaviour)
            {
                case Behaviour::Momentary:
                    this->_process_momentary(input);
                    break;
                case Behaviour::Latching:
                    this->_process_latching(input);
                    break;
                default: break;
            }
        }

        /** Sets the processing behaviour. */
        void set_behaviour(Flag::Behaviour behaviour)
        {
            this->behaviour = behaviour;
        }
        /** @returns The current behaviour. */
        Flag::Behaviour get_behaviour() const
        {
            return this->behaviour;
        }

        /** @returns `true` if the flag is currently high. */
        bool is_high() const
        {
            return this->state;
        }
        /** @returns `true` if the flag has been newly raised to high. */
        bool is_rising() const
        {
            return this->state && !this->prev_state;
        }
        /** @returns `true` if the flag has newly fallen to low. */
        bool is_falling() const
        {
            return !this->state && this->prev_state;
        }
        /** @returns `true` if the flag changed state on last process. */
        bool has_changed() const
        {
            return this->state != this->prev_state;
        }
        /** @returns setting flag state high */
        void set_high()
        {
            this->state = bool(1);
            this->prev_state = this->state;
        }
        /** @returns setting flag state low */
        void set_low()
        {
            this->state = bool(0);
        }

    private:
        void _process_momentary(bool input)
        {
            this->prev_state = this->state;
            this->state = input;
            this->prev_input = input;
        }
        void _process_latching(bool input)
        {
            this->prev_state = this->state;
            if (input && !this->prev_input)
                this->state = !this->state;
            this->prev_input = input;
        }

        /** Current behaviour model. */
        Behaviour behaviour;

        /** Current state. */
        bool state;
        /** Previous state. */
        bool prev_state;

        /** Previous input, for latching behaviour. */
        bool prev_input;
};


/** Continuous parameter abstraction class.
 * Template paramaters:
 * @param T Data type of the output and *all* internal processing, e.g. `float`.
 * @param InputT Data type of the input to be processed, e.g. `uint16_t`.
 * @param Processors (Compile-time) Variable number of data processing classes.
 * @note The input value of type `InputT` is converted to type `T` before being
 * passed to the first processor.
 */
template<
    class T, class InputT,
    class... Processors
>
class Parameter
{
    public:
        Parameter(Processors&&... processors):
        output{T{}},
        prev_output{T{}},
        processors{std::make_tuple(processors...)}
        {}

        ~Parameter() = default;

        /** Passes input through the given processors sequentially. */
        T process(InputT input)
        {
            this->prev_output = this->output;
            this->output = this->_process<0>(static_cast<T>(input));
            return this->output;
        }

        /** Sets the given processor to @a processor. */
        template<class Processor>
        void set_processor(const Processor& processor)
        {
            std::get<Processor>(this->processors) = processor;
        }

        /** @returns A reference to the specified processor. */
        template<class Processor>
        Processor& get_processor()
        {
            return std::get<Processor>(this->processors);
        }
        template<class Processor>
        const Processor& get_processor() const
        {
            return std::get<Processor>(this->processors);
        }

        /** @returns `true` if the output changed on last process. */
        bool has_changed() const
        {
            if IDSP_CONSTEXPR_SINCE_CXX17 (std::is_integral<T>::value)
            {
                return this->output != this->prev_output;
            }
            else
            {
                constexpr T threshold = 1e-6;
                return std::abs(this->output - this->prev_output) > threshold;
            }
        }

        /** @returns The value produced by last process. */
        T get_output() const
        {
            return this->output;
        }

    private:
        template<size_t Index>
        T _process(T in)
        {
            if constexpr (Index < sizeof...(Processors))
            {
                auto& proc = std::get<Index>(this->processors);
                const T output = proc.process(in);
                return this->_process<Index + 1>(output);
            }
            else
            {
                return in;
            }
        }

        T output;
        T prev_output;

        std::tuple<Processors...> processors;
};

namespace paramproc
{
    namespace smoothing
    {
        template<class T>
        class None
        {
            public:
                None() = default;
                ~None() = default;

            T process(T input)
            {
                return input;
            }
        };

        template<class T>
        class SlewExponential
        {
            public:
                SlewExponential(float rate):
                value{T{}},
                rate{rate}
                {}

                ~SlewExponential() = default;

                T process(T input)
                {
                    const T val = idsp::interpolate_2(this->rate, this->value, input);
                    this->value = val;
                    return val;
                }

            private:
                T value;

                float rate;
        };

        template<class T>
        class SlewLinear
        {
            public:
                SlewLinear(T step):
                value{T{}},
                step{step}
                {}

                ~SlewLinear() = default;

                T process(T input)
                {
                    const T sign = input >= this->value ? T(1) : T(-1);
                    const T add = sign * std::min(this->step, std::abs(input - this->value));
                    const T val = this->value + add;
                    this->value = val;
                    return val;
                }

            private:
                T value;

                T step;
        };
    }

    namespace hysterisis
    {
        template<class T>
        class None
        {
            public:
                None() = default;
                ~None() = default;

                T process(T input)
                {
                    this->prev = this->value;
                    this->value = input;
                    return this->value;
                }

            private:
                T value;
                T prev;
        };

        template<class T>
        class ChangeThreshold
        {
            public:
                ChangeThreshold(T threshold):
                value{T{}},
                threshold{threshold}
                {}

                ~ChangeThreshold() = default;

                T process(T input)
                {
                    if (std::abs(input - this->value) >= this->threshold)
                    {
                        this->value = input;
                    }
                    return this->value;
                }

            private:
                T value;

                T threshold;
        };
    }

    namespace scaling
    {
        template<class T>
        class None
        {
            public:
                None() = default;
                ~None() = default;

                T process(T input) const
                {
                    return input;
                }
        };

        template<class T>
        class LinearMapping
        {
            public:
                LinearMapping(
                    T in_min, T in_max,
                    T out_min = 0.f, T out_max = 1.f
                ):
                in_min{in_min}, in_max{in_max},
                out_min{out_min}, out_max{out_max}
                {}

                T process(T input)
                {
                    return idsp::rescale<T>(
                        input,
                        this->in_min, this->in_max,
                        this->out_min, this->out_max
                    );
                }

            private:
                T in_min;
                T in_max;
                T out_min;
                T out_max;
        };
    }

    namespace condition
    {
        template<class T>
        class None
        {
            public:
                None() = default;
                ~None() = default;

                T process(T input) const
                {
                    return input;
                }
        };

        template<class T>
        class ScaleClamp
        {
            public:
                ScaleClamp(
                    T in_min, T in_max,
                    T out_min = 0.f, T out_max = 1.f
                ):
                in_min{in_min}, in_max{in_max},
                out_min{out_min}, out_max{out_max}
                {}

                T process(T input)
                {
                    T val = idsp::rescale<T>(input,in_min,in_max,out_min,out_max);
                    return idsp::clamp<T>(val,out_min,out_max);
                }
            private:
                T in_min;
                T in_max;
                T out_min;
                T out_max;

        };

        template<class T>
        class Midpoint
        {
            public:
                Midpoint(
                    T mid_val = 2048, T mid_notch = 0
                ):
                mid_val{mid_val}, mid_notch{mid_notch}
                {}

                T process(T input)
                {
                    T val = input + 1;
                    T low_notch = mid_val - mid_notch;
                    T high_notch = mid_val + mid_notch;
                    if(val > high_notch)
                        val = idsp::rescale<T>(val,this->mid_val,4094,2049,4095);
                    else if(val < low_notch)
                        val = idsp::rescale<T>(val,1,this->mid_val,1,2047);
                    else
                        val = 2048;
                    return idsp::clamp<T>(val - 1,0,4095);
                }

                void setMidVal(T mid) { this->mid_val = mid; }
                void setMidNotch(T notch) { this->mid_notch = notch; }

            private:
                T mid_val;
                T mid_notch;

        };

    }
} // namespace paramproc

} // namespace idsp

#endif
