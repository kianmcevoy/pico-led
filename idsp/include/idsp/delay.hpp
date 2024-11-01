#ifndef IDSP_DELAY_H
#define IDSP_DELAY_H

#include "idsp/buffer_interface.hpp"
#include "idsp/buffer_types.hpp"
#include "idsp/ringbuffer.hpp"
#include "idsp/wrapper.hpp"
#include "idsp/modulation.hpp"
#include "idsp/reverb_toolkit.hpp"
#include <vector>
#include <array>

namespace idsp
{
    template<size_t S>
    class VarispeedDelay
    {
        public:
            VarispeedDelay() :
            slew_amount{2400.f},
            slew_position{0.f},
            freeze{false}
            {}
            ~VarispeedDelay() = default;

            void process(const BufferInterface& input, BufferInterface& output)
            {
                for(size_t i = 0; i < input.size(); i++)
                    output[i] = _process(input[i]);
            }

            template<size_t N>
            void process_for(const BufferInterface& input, BufferInterface& output)
            {
                for(size_t i = 0; i < N; i++)
                    output[i] = _process(input[i]);
            }

            inline void set_time(const float f) {delay_time = min<float>(f, delay->get_size());}

            constexpr
            inline void set_slew_amount(const float slew_samples) {slew_amount = max(slew_samples, 1.f);}

            constexpr
            inline void set_freeze(const bool b) {freeze = b;}

        private:
            Sample _process(const Sample input)
            {
                slew_position += ((delay_time-slew_position)/slew_amount);
                Sample output = delay->read_offset_smooth_safe(slew_position);
                if(!freeze)delay->write(input);
                return output;
            }
            Delay<SampleBufferStatic<S>> delay;
            float delay_time;
            float slew_amount;
            float slew_position;
            bool freeze;
    };
} // namespace idsp

#endif