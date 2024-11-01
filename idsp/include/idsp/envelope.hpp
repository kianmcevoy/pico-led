#ifndef IDSP_ENVELOPE_H
#define IDSP_ENVELOPE_H

#include "idsp/buffer_interface.hpp"
#include "idsp/constants.hpp"
#include "idsp/functions.hpp"
#include "idsp/std_helpers.hpp"


namespace idsp
{
    enum class EnvelopeState
    {
        IDLE,
        ATTACK,
        DECAY,
        SUSTAIN,
        HOLD,
        RELEASE,
        EOC
    };

    enum class EnvelopeMode
    {
        GATE,
        TRIGGER,
        LOOPING
    };

    enum class EnvelopeType
    {
        AR,
        ASR,
        AHR,
        ADSR
    };

    class Envelope
    {
        public:

            using State = idsp::EnvelopeState;
            using Mode = idsp::EnvelopeMode;

            Envelope():
            attack_time{480.f},
            decay_time{480.f},
            hold_time{48000.f},
            hold_time_remaining{48000.f},
            sustain_level{0.68f},
            release_time{480.f},
            x{0.f},
            shape{0.f},
            state{State::IDLE},
            mode{Mode::TRIGGER},
            type{EnvelopeType::ADSR},
            prev_gate{false},
            retrigger{false}
            {}

            constexpr
            inline void set_attack(float f) {attack_time = max(f, 1.f);}

            constexpr
            inline void set_decay(float time_in_samples) {decay_time =  max(time_in_samples, 1.f);}

            constexpr
            inline void set_sustain(float level) {sustain_level = idsp::clamp(level, 0.f, 1.f);}

            constexpr
            inline void set_hold(float time_in_samples){ hold_time = time_in_samples; }

            constexpr
            inline void set_release(float f) {release_time = max(f, 1.f);}

            constexpr
            inline void set_shape(float f) {shape = idsp::clamp(f, 0.f, 1.f);}

            constexpr
            inline void set_retrigger(bool r)
            {
                if (r && (state != State::ATTACK))
                {
                    state = State::ATTACK;
                    if (mode == Mode::LOOPING) x = 0.f;
                }
            }

            constexpr
            inline void set_mode(Mode new_mode) {mode = new_mode;}

            constexpr
            inline void set_type(EnvelopeType new_type) {type = new_type;}

            constexpr
            inline void set_state(State new_state)
            {
                /*if(new_state == State::ATTACK) x = 0.f; Uncomment this to have the envelope reset to 0 when state is set to attack
                else */if(new_state == State::DECAY) x = 1.f;
                else if(new_state == State::SUSTAIN) x = sustain_level;
                else if(new_state == State::RELEASE) x = sustain_level;
                else if(new_state == State::IDLE) x = 0.f;
                state = new_state;
            }

            constexpr
            inline State get_state() {return state;}

            constexpr
            inline Mode get_mode() { return mode; }

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            inline void process_for(BufferInterface& modulation, bool gate)
            {
                for(size_t i = 0; i < N; i++)
                    modulation[i] = process(gate);
            }

            Sample process(bool gate)
            {
                if (gate && !prev_gate) state = State::ATTACK;

                switch (state)
                {
                    case State::ATTACK:
                        if((x >= 0.995f || ((type == EnvelopeType::ASR) && (x >= sustain_level)) || (!gate && (mode == Mode::GATE))))
                        {
                            if (type == EnvelopeType::AR)  state = State::RELEASE;
                            if (type == EnvelopeType::ASR) { mode != Mode::LOOPING ? state = State::SUSTAIN : state = State::RELEASE; }
                            if (type == EnvelopeType::AHR) { state = State::HOLD; hold_time_remaining = hold_time; }
                            if (type == EnvelopeType::ADSR){ mode != Mode::LOOPING ? state = State::DECAY : state = State::RELEASE; }
                        }
                        else x += _get_step(1.f - x, attack_time, shape);
                        break;

                    case State::DECAY:
                        if(gate)
                        {
                            if(x <= (sustain_level+0.005f)) state = State::SUSTAIN;
                            else  x += _get_step(sustain_level - x, decay_time, shape);
                        }
                        else state = State::RELEASE;
                        break;

                    case State::HOLD:
                        if(hold_time_remaining <= 0) state = State::RELEASE;
                        else hold_time_remaining--;
                        break;

                    case State::SUSTAIN:
                        if(gate)
                        {
                            x = sustain_level;
                        }
                        else state = State::RELEASE;
                        break;

                    case State::RELEASE:
                        if(x <= 1e-3f) state = State::EOC;
                        else x += _get_step(-x, release_time, shape);
                        break;

                    case State::EOC:
                        x = 0.f;
                        state = State::IDLE;
                        break;

                    case State::IDLE:
                        if (mode == Mode::LOOPING) state = State::ATTACK;
                        x = 0.f;
                        break;

                    default:
                        state = State::IDLE;
                        break;
                }
                prev_gate = gate;
                retrigger = false;
                return clamp(x, 0.f, 1.f);
            }

            private:

            float _get_step(float delta, float tau, float shape)
            {
                float linear = sgn(delta) * 1.f / tau;
                float exp = static_cast<float>(M_E) * delta / (0.8f*tau);
                return interpolate_2(shape, linear, exp);
            }

            float attack_time;
            float decay_time;
            float hold_time;           /* in samples */
            float hold_time_remaining; /* ^^^^^^^^^^ */
            float sustain_level;
            float release_time;
            Sample x;
            float shape;
            State state;
            EnvelopeMode mode;
            EnvelopeType type;
            bool prev_gate;
            bool retrigger;
    };

/*

    This version of AR uses a bool as the input for process instead of a float.

    class AR
    {
        public:

            using State = idsp::EnvelopeState;
            using Mode = idsp::EnvelopeMode;

            AR():
            attack_time{480.f},
            release_time{480.f},
            x{0.f},
            shape{0.f},
            state{State::IDLE},
            mode{Mode::TRIGGER},
            prev_gate{false},
            retrigger{false}
            {}

            constexpr
            inline void set_attack(float f) {attack_time = max(f, 1.f);}

            constexpr
            inline void set_release(float f) {release_time = max(f, 1.f);}

            constexpr
            inline void set_shape(float f) {shape = idsp::clamp(f, 0.f, 1.f);}

            constexpr
            inline void set_retrigger(bool r) { if (r) retrigger = false; }

            constexpr
            inline void set_mode(Mode new_mode) {mode = new_mode;}

            constexpr
            inline void set_state(State new_state) { state = new_state; }

            constexpr
            inline State get_state() {return state;}

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            inline void process_for(BufferInterface& modulation, float gate)
            {
                for(size_t i = 0; i < N; i++)
                    modulation[i] = process(gate);
            }

            Sample process(float gate)
            {
                if (gate && !prev_gate) state = State::ATTACK;

                switch (state)
                {
                    case State::ATTACK:
                        if((x >= 0.995f || (!gate && (mode == Mode::GATE))))
                        {
                            state = State::RELEASE;
                        }
                        else x += _get_step(1.f - x, attack_time, shape);
                        break;

                    case State::RELEASE:
                        if(x <= 1e-3f) state = State::EOC;
                        else x += _get_step(-x, release_time, shape);
                        break;

                    case State::EOC:
                        x = 0.f;
                        state = State::IDLE;
                        break;

                    case State::IDLE:
                        if (mode == Mode::LOOPING) state = State::ATTACK;
                        x = 0.f;
                        break;

                    default:
                        state = State::IDLE;
                        break;
                }
                prev_gate = gate;
                return clamp(x, 0.f, 1.f);
            }

            private:

            float _get_step(float delta, float tau, float shape)
            {
                float linear = sgn(delta) * 1.f / tau;
                float exp = static_cast<float>(M_E) * delta / (0.8f*tau);
                return interpolate_2(shape, linear, exp);
            }

            float attack_time;
            float release_time;
            Sample x;
            float shape;
            State state;
            EnvelopeMode mode;
            EnvelopeType type;
            bool prev_gate;
            bool retrigger;
    };
*/

    class ASR
    {
           public:

            using State = idsp::EnvelopeState;
            using Mode = idsp::EnvelopeMode;

            ASR():
            attack_time{480.f},
            decay_time{480.f},
            sustain_level{0.68f},
            release_time{480.f},
            x{0.f},
            shape{0.f},
            state{State::IDLE},
            mode{Mode::TRIGGER},
            prev_gate{false},
            retrigger{false}
            {}

            constexpr
            inline void set_attack(float f) {attack_time = max(f, 1.f);}

            constexpr
            inline void set_decay(float time_in_samples) {decay_time =  max(time_in_samples, 1.f);}

            constexpr
            inline void set_sustain(float level) {sustain_level = idsp::clamp(level, 0.f, 1.f);}

            constexpr
            inline void set_release(float f) {release_time = max(f, 1.f);}

            constexpr
            inline void set_shape(float f) {shape = idsp::clamp(f, 0.f, 1.f);}

            constexpr
            inline void set_retrigger(bool r) { if (r) retrigger = false; }

            constexpr
            inline void set_mode(Mode new_mode) {mode = new_mode;}

            constexpr
            inline void set_state(State new_state) { state = new_state; }

            constexpr
            inline State get_state() {return state;}

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            inline void process_for(BufferInterface& modulation, bool gate)
            {
                for(size_t i = 0; i < N; i++)
                    modulation[i] = process(gate);
            }

            Sample process(bool gate)
            {
                if (gate && !prev_gate) state = State::ATTACK;

                switch (state)
                {
                    case State::ATTACK:
                        if(x >= 0.995f || ((x >= sustain_level)) || (!gate && (mode == Mode::GATE)))
                        {
                            mode != Mode::LOOPING ? state = State::SUSTAIN : state = State::RELEASE;
                        }
                        else x += _get_step(1.f - x, attack_time, shape);
                        break;

                    case State::DECAY:
                        if(gate)
                        {
                            if(x <= (sustain_level+0.005f)) state = State::SUSTAIN;
                            else  x += _get_step(sustain_level - x, decay_time, shape);
                        }
                        else state = State::RELEASE;
                        break;

                    case State::SUSTAIN:
                        if(gate)
                        {
                            x = sustain_level;
                        }
                        else state = State::RELEASE;
                        break;

                    case State::RELEASE:
                        if(x <= 1e-3f) state = State::EOC;
                        else x += _get_step(-x, release_time, shape);
                        break;

                    case State::EOC:
                        x = 0.f;
                        state = State::IDLE;
                        break;

                    case State::IDLE:
                        if (mode == Mode::LOOPING) state = State::ATTACK;
                        x = 0.f;
                        break;

                    default:
                        state = State::IDLE;
                        break;
                }
                prev_gate = gate;
                return clamp(x, 0.f, 1.f);
            }

            private:

            float _get_step(float delta, float tau, float shape)
            {
                float linear = sgn(delta) * 1.f / tau;
                float exp = static_cast<float>(M_E) * delta / (0.8f*tau);
                return interpolate_2(shape, linear, exp);
            }

            float attack_time;
            float decay_time;
            float sustain_level;
            float release_time;
            Sample x;
            float shape;
            State state;
            EnvelopeMode mode;
            bool prev_gate;
            bool retrigger;
    };

    class AHR
    {
        public:

            using State = idsp::EnvelopeState;
            using Mode = idsp::EnvelopeMode;

            AHR():
            attack_time{480.f},
            hold_time{48000.f},
            hold_time_remaining{48000.f},
            sustain_level{0.68f},
            release_time{480.f},
            x{0.f},
            shape{0.f},
            state{State::IDLE},
            mode{Mode::TRIGGER},
            prev_gate{false},
            retrigger{false}
            {}

            constexpr
            inline void set_attack(float f) {attack_time = max(f, 1.f);}

            constexpr
            inline void set_sustain(float level) {sustain_level = idsp::clamp(level, 0.f, 1.f);}

            constexpr
            inline void set_hold(float time_in_samples){ hold_time = time_in_samples; }

            constexpr
            inline void set_release(float f) {release_time = max(f, 1.f);}

            constexpr
            inline void set_shape(float f) {shape = idsp::clamp(f, 0.f, 1.f);}

            constexpr
            inline void set_retrigger(bool r) { if (r) retrigger = false; }

            constexpr
            inline void set_mode(Mode new_mode) {mode = new_mode;}

            constexpr
            inline void set_state(State new_state) { state = new_state; }

            constexpr
            inline State get_state() {return state;}

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            inline void process_for(BufferInterface& modulation, bool gate)
            {
                for(size_t i = 0; i < N; i++)
                    modulation[i] = process(gate);
            }

            Sample process(bool gate)
            {
                if (gate && !prev_gate) state = State::ATTACK;

                switch (state)
                {
                    case State::ATTACK:
                        if(x >= 0.995f || ((!gate && (mode == Mode::GATE))))
                        {
                            state = State::HOLD;
                            hold_time_remaining = hold_time;
                        }
                        else x += _get_step(1.f - x, attack_time, shape);
                        break;

                    case State::HOLD:
                        if(hold_time_remaining <= 0) state = State::RELEASE;
                        else hold_time_remaining--;
                        break;

                    case State::SUSTAIN:
                        if(gate)
                        {
                            x = sustain_level;
                        }
                        else state = State::RELEASE;
                        break;

                    case State::RELEASE:
                        if(x <= 1e-3f) state = State::EOC;
                        else x += _get_step(-x, release_time, shape);
                        break;

                    case State::EOC:
                        x = 0.f;
                        state = State::IDLE;
                        break;

                    case State::IDLE:
                        if (mode == Mode::LOOPING) state = State::ATTACK;
                        x = 0.f;
                        break;

                    default:
                        state = State::IDLE;
                        break;
                }
                prev_gate = gate;
                return clamp(x, 0.f, 1.f);
            }

            private:

            float _get_step(float delta, float tau, float shape)
            {
                float linear = sgn(delta) * 1.f / tau;
                float exp = static_cast<float>(M_E) * delta / (0.8f*tau);
                return interpolate_2(shape, linear, exp);
            }

            float attack_time;
            float hold_time;           /* in samples */
            float hold_time_remaining; /* ^^^^^^^^^^ */
            float sustain_level;
            float release_time;
            Sample x;
            float shape;
            State state;
            EnvelopeMode mode;
            bool prev_gate;
            bool retrigger;
    };


    class ADSR
    {
        public:

            using State = idsp::EnvelopeState;
            using Mode = idsp::EnvelopeMode;

            ADSR():
            attack_time{480.f},
            decay_time{480.f},
            sustain_level{0.68f},
            release_time{480.f},
            x{0.f},
            shape{0.f},
            state{State::IDLE},
            mode{Mode::TRIGGER},
            prev_gate{false},
            retrigger{false}
            {}

            constexpr
            inline void set_attack(float f) {attack_time = max(f, 1.f);}

            constexpr
            inline void set_decay(float time_in_samples) {decay_time =  max(time_in_samples, 1.f);}

            constexpr
            inline void set_sustain(float level) {sustain_level = idsp::clamp(level, 0.f, 1.f);}

            constexpr
            inline void set_release(float f) {release_time = max(f, 1.f);}

            constexpr
            inline void set_shape(float f) {shape = idsp::clamp(f, 0.f, 1.f);}

            constexpr
            inline void set_retrigger(bool r) { if (r) retrigger = false; }

            constexpr
            inline void set_mode(Mode new_mode) {mode = new_mode;}

            constexpr
            inline void set_state(State new_state) { state = new_state; }

            constexpr
            inline State get_state() {return state;}

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            inline void process_for(BufferInterface& modulation, bool gate)
            {
                for(size_t i = 0; i < N; i++)
                    modulation[i] = process(gate);
            }

            Sample process(bool gate)
            {
                if (gate && !prev_gate) state = State::ATTACK;

                switch (state)
                {
                    case State::ATTACK:
                        if((x >= 0.995f || (!gate && (mode == Mode::GATE))))
                        {
                            if (mode != Mode::LOOPING) state = State::DECAY;
                            else state = State::RELEASE;
                        }
                        else x += _get_step(1.f - x, attack_time, shape);
                        break;

                    case State::DECAY:
                        if(gate)
                        {
                            if(x <= (sustain_level+0.005f)) state = State::SUSTAIN;
                            else  x += _get_step(sustain_level - x, decay_time, shape);
                        }
                        else state = State::RELEASE;
                        break;

                    case State::SUSTAIN:
                        if(gate)
                        {
                            x = sustain_level;
                        }
                        else state = State::RELEASE;
                        break;

                    case State::RELEASE:
                        if(x <= 1e-3f) state = State::EOC;
                        else x += _get_step(-x, release_time, shape);
                        break;

                    case State::EOC:
                        x = 0.f;
                        state = State::IDLE;
                        break;

                    case State::IDLE:
                        if (mode == Mode::LOOPING) state = State::ATTACK;
                        x = 0.f;
                        break;

                    default:
                        state = State::IDLE;
                        break;
                }
                prev_gate = gate;
                return clamp(x, 0.f, 1.f);
            }

            private:

            float _get_step(float delta, float tau, float shape)
            {
                float linear = sgn(delta) * 1.f / tau;
                float exp = static_cast<float>(M_E) * delta / (0.8f*tau);
                return interpolate_2(shape, linear, exp);
            }

            float attack_time;
            float decay_time;
            float sustain_level;
            float release_time;
            Sample x;
            float shape;
            State state;
            EnvelopeMode mode;
            bool prev_gate;
            bool retrigger;
    };

    /** Uses a Sample (float) as the input for process  */

    class AR
    {
        public:
            using State = idsp::EnvelopeState;
            using Mode = idsp::EnvelopeMode;

            AR():
            attack_time{480.f},
            release_time{480.f},
            x{0.f},
            shape{0.f},
            state{State::IDLE},
            mode{Mode::TRIGGER}
            {}

            constexpr
            inline void set_attack(float f) {attack_time = max(f, 1.f);}

            constexpr
            inline void set_release(float f) {release_time = max(f, 1.f);}

            constexpr
            inline void set_shape(float f) {shape = f;}

            constexpr
            inline void set_mode(EnvelopeMode new_mode) {mode = new_mode;}

            constexpr
            inline void set_state(State new_state) { state = new_state; }

            constexpr
            inline State get_state() {return state;}

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            inline void process_for(BufferInterface& modulation, float input)
            {
                for(size_t i = 0; i < N; i++)
                    modulation[i] = process(input);
            }

            Sample process(const Sample input)
            {
                switch (state)
                {
                    case State::ATTACK:
                        if((x >= 0.995f || (((input - x) < 0.01f) && (mode == Mode::GATE))))
                        {
                            state = State::RELEASE;
                        }
                        else x += _get_step(1.f - x, attack_time, shape);
                        break;

                    case State::RELEASE:
                        if(x <= 1e-3f) state = State::EOC;
                        else x += _get_step(-x, release_time, shape);
                        break;

                    case State::EOC:
                        x = 0.f;
                        state = State::IDLE;
                        break;

                    case State::IDLE:
                        if (mode == Mode::LOOPING) state = State::ATTACK;
                        x = 0.f;
                        break;

                    default:
                        state = State::IDLE;
                        break;
                }
                return clamp(x, 0.f, 1.f);
            }

        private:

            float _get_step(float delta, float tau, float shape)
            {
                float linear = sgn(delta) * 1.f / tau;
                float exp = static_cast<float>(M_E) * delta / (0.8f*tau);
                return interpolate_2(shape, linear, exp);
            }

            float attack_time;
            float release_time;
            Sample x;
            float shape;
            State state;
            EnvelopeMode mode;
            EnvelopeType type;
    };

    class EnvelopeFollower
    {
        public:

            using State = idsp::EnvelopeState;
            using Mode = idsp::EnvelopeMode;

            EnvelopeFollower()
            {
                env.set_mode(Mode::GATE);
            }

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            void process_for(const BufferInterface& input, BufferInterface& output)
            {
                for(size_t i = 0; i < N; i++)
                    output[i] = _process(input[i]);
            }

            Sample process(const Sample input)
            {
               return _process(input);
            }

            inline void set_attack(float f) {env.set_attack(max(f, 128.f));}

            inline void set_release(float f) {env.set_release(max(f, 128.f));}

            inline void set_shape(float f) {env.set_shape(f);}

        private:
            Sample _process(const Sample input)
            {
                 //rectify the input
                Sample in_sample = fabs(input);
                //square for rms
                in_sample *= in_sample;
                //use env as voltage controlled slew limiter
                return env.process(in_sample);
            }

            Envelope env;
    };

} // namespace idsp


#endif
