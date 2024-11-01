#ifndef IDSP_MODULATION_H
#define IDSP_MODULATION_H

#include "idsp/buffer_interface.hpp"
#include "idsp/envelope.hpp"
#include "idsp/oscillator.hpp"
#include "idsp/random.hpp"

namespace idsp
{
    class Ramp
    {
        public:

            enum State
            {
                RISE,
                HOLD,
                EOC
            };

            Ramp() :
            x{0},
            state{State::HOLD},
            duration{0},
            time_remaining{0}
            {}

            Ramp(float time) :
            x{0},
            state{State::HOLD},
            duration{time},
            time_remaining{0}
            {}

            /** Set ramp duration in samples */
            inline void set_time(float time_in_samples){duration = time_in_samples;}

            inline void set_state(State s) {state = s;}

            inline State get_state() {return state;}

            inline void trigger()
            {
                x = 0.f;
                state = HOLD;
                time_remaining = duration;
            }

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            void process_for(BufferInterface& modulation)
            {
                for(size_t i = 0; i < N; i++)
                    modulation[i] = process();
            }

            float process()
            {
                if(x < 1.f && state != EOC)
                {
                    state = RISE;
                    float distance = 1.f - x;
                    float step = distance / time_remaining;
                    x += step;
                    time_remaining--;
                }
                else
                {
                    x = 1.f;
                    state = EOC;
                };
                return clamp(x, 0.f, 1.f);
            }

        private:
            float x;
            State state;
            float duration;
            float time_remaining;
    };
}//namespace idsp

#endif
