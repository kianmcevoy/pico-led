#ifndef IDSP_FILTER_H
#define IDSP_FILTER_H

#include "idsp/buffer_interface.hpp"
#include "idsp/functions.hpp"

#include <algorithm>
#include <cmath>

namespace idsp
{
    /** Single-pole multi-mode filter. */
    class OnepoleFilter
    {
        public:
            enum class Type
            {
                Lowpass,
                Highpass,
                Allpass,
            };

            constexpr
            OnepoleFilter(Type filter_type, Sample cutoff):
            type{filter_type},
            coef{Sample(1) / (Sample(M_PI) * cutoff)},
            xState{Sample{}},
            yState{Sample{}}
            {}

            constexpr
            OnepoleFilter(Type filter_type):
            OnepoleFilter(filter_type, Sample(0.1))
            {}

            IDSP_CONSTEXPR_SINCE_CXX20
            ~OnepoleFilter() = default;

            /** Set normalised cutoff/center frequency. */
            IDSP_CONSTEXPR_SINCE_CXX14
            void set_cutoff(Sample f)
            {
                this->coef = Sample(1) / (pi * f);
            }

            /** Filter block processor. */
            IDSP_CONSTEXPR_SINCE_CXX14
            void process(const BufferInterface& input, BufferInterface& output)
            {
                this->_process_block(input, output);
            }

            /** Filter sample processor. */
            IDSP_CONSTEXPR_SINCE_CXX14
            Sample process(Sample input)
            {
                return this->_process_sample(input);
            }

        private:
            IDSP_CONSTEXPR_SINCE_CXX14
            Sample _process_lowpass(Sample x)
            {
                this->yState = (x + this->xState - this->yState * (Sample(1) - this->coef)) / (Sample(1) + this->coef);
                this->xState = x;
                return this->yState;
            }
            IDSP_CONSTEXPR_SINCE_CXX14
            Sample _process_highpass(Sample x)
            {
                this->yState = (x + this->xState - this->yState * (Sample(1) - this->coef)) / (Sample(1) + this->coef);
                this->xState = x;
                return x - this->yState;
            }
            IDSP_CONSTEXPR_SINCE_CXX14
            Sample _process_allpass(Sample x)
            {
                this->yState = (x + this->xState - this->yState * (Sample(1) - this->coef)) / (Sample(1) + this->coef);
                this->xState = x;
                return this->yState - (x - this->yState);
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            Sample _process_sample(Sample input)
            {
                switch (this->type)
                {
                    case Type::Lowpass:
                        return this->_process_lowpass(input);
                        break;
                    case Type::Highpass:
                        return this->_process_highpass(input);
                        break;
                    case Type::Allpass:
                        return this->_process_allpass(input);
                        break;
                    default:
                        return input;
                        break;
                }
            }
            IDSP_CONSTEXPR_SINCE_CXX14
            void _process_block(const BufferInterface& input, BufferInterface& output)
            {
                const size_t size = idsp::min(input.size(), output.size());
                switch (this->type)
                {
                    case Type::Lowpass:
                        for (size_t i = 0; i < size; i++)
                            output[i] = this->_process_lowpass(input[i]);
                        break;
                    case Type::Highpass:
                        for (size_t i = 0; i < size; i++)
                            output[i] = this->_process_highpass(input[i]);
                        break;
                    case Type::Allpass:
                        for (size_t i = 0; i < size; i++)
                            output[i] = this->_process_allpass(input[i]);
                        break;
                    default:
                        std::copy_n(input.begin(), size, output.begin());
                        break;
                }
            }

            Type type;

            Sample coef;
            Sample xState;
            Sample yState;
    };

    /** Biquadratic multi-mode filter. */
    class BiquadFilter
    {
        public:
            enum class Type
            {
                Lowpass1Pole,
                Highpass1Pole,
                Lowpass,
                Highpass,
                Lowshelf,
                Highshelf,
                Bandpass,
                Peak,
                Notch
            };

            IDSP_CONSTEXPR_SINCE_CXX14
            BiquadFilter(Type filter_type):
            type{filter_type},
            a{},
            b{},
            xState{}
            {
                this->a.fill(0);
                this->b.fill(0);
                this->xState.fill(0);
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            BiquadFilter(Type filter_type, Sample f, Sample Q, Sample V = Sample(1)):
            BiquadFilter(filter_type)
            {
                this->set_parameters(f, Q, V);
            }

            IDSP_CONSTEXPR_SINCE_CXX20
            ~BiquadFilter() = default;

            /** Set the filter's parameters.
             * @param f The filter cutoff/center normalised frequency.
             * @param Q Filter resonance.
             */
            void set_parameters(Sample f, Sample Q, Sample V = Sample(1))
            {
                constexpr Sample sqrt2 = M_SQRT2;
                const Sample K = std::tan(pi * f);

                switch (type) {
                    case Type::Lowpass1Pole: {
                        this->a[0] = -std::exp(-Sample(2) * pi * f);
                        this->a[1] = Sample(0);
                        this->b[0] = Sample(1) + this->a[0];
                        this->b[1] = Sample(0);
                        this->b[2] = Sample(0);
                    } break;

                    case Type::Highpass1Pole: {
                        this->a[0] = std::exp(-Sample(2) * pi * (0.5f - f));
                        this->a[1] = Sample(0);
                        this->b[0] = Sample(1) - this->a[0];
                        this->b[1] = Sample(0);
                        this->b[2] = Sample(0);
                    } break;

                    case Type::Lowpass: {
                        const Sample norm = Sample(1) / (Sample(1) + K / Q + K * K);
                        this->b[0] = K * K * norm;
                        this->b[1] = Sample(2) * this->b[0];
                        this->b[2] = this->b[0];
                        this->a[0] = Sample(2) * (K * K - Sample(1)) * norm;
                        this->a[1] = (Sample(1) - K / Q + K * K) * norm;
                    } break;

                    case Type::Highpass: {
                        const Sample norm = Sample(1) / (Sample(1) + K / Q + K * K);
                        this->b[0] = norm;
                        this->b[1] = -Sample(2) * this->b[0];
                        this->b[2] = this->b[0];
                        this->a[0] = Sample(2) * (K * K - Sample(1)) * norm;
                        this->a[1] = (Sample(1) - K / Q + K * K) * norm;

                    } break;

                    case Type::Lowshelf: {
                        const Sample sqrtV = std::sqrt(V);
                        if (V >= Sample(1)) {
                            const Sample norm = Sample(1) / (Sample(1) + sqrt2 * K + K * K);
                            this->b[0] = (Sample(1) + sqrt2 * sqrtV * K + V * K * K) * norm;
                            this->b[1] = Sample(2) * (V * K * K - Sample(1)) * norm;
                            this->b[2] = (Sample(1) - sqrt2 * sqrtV * K + V * K * K) * norm;
                            this->a[0] = Sample(2) * (K * K - Sample(1)) * norm;
                            this->a[1] = (Sample(1) - sqrt2 * K + K * K) * norm;
                        }
                        else {
                            const Sample norm = Sample(1) / (Sample(1) + sqrt2 / sqrtV * K + K * K / V);
                            this->b[0] = (Sample(1) + sqrt2 * K + K * K) * norm;
                            this->b[1] = Sample(2) * (K * K - 1) * norm;
                            this->b[2] = (Sample(1) - sqrt2 * K + K * K) * norm;
                            this->a[0] = Sample(2) * (K * K / V - Sample(1)) * norm;
                            this->a[1] = (Sample(1) - sqrt2 / sqrtV * K + K * K / V) * norm;
                        }
                    } break;

                    case Type::Highshelf: {
                        const Sample sqrtV = std::sqrt(V);
                        if (V >= Sample(1)) {
                            const Sample norm = Sample(1) / (Sample(1) + sqrt2 * K + K * K);
                            this->b[0] = (V + sqrt2 * sqrtV * K + K * K) * norm;
                            this->b[1] = Sample(2) * (K * K - V) * norm;
                            this->b[2] = (V - sqrt2 * sqrtV * K + K * K) * norm;
                            this->a[0] = Sample(2) * (K * K - Sample(1)) * norm;
                            this->a[1] = (Sample(1) - sqrt2 * K + K * K) * norm;
                        }
                        else {
                            const Sample norm = Sample(1) / (Sample(1) / V + sqrt2 / sqrtV * K + K * K);
                            this->b[0] = (Sample(1) + sqrt2 * K + K * K) * norm;
                            this->b[1] = Sample(2) * (K * K - Sample(1)) * norm;
                            this->b[2] = (Sample(1) - sqrt2 * K + K * K) * norm;
                            this->a[0] = Sample(2) * (K * K - Sample(1) / V) * norm;
                            this->a[1] = (Sample(1) / V - sqrt2 / sqrtV * K + K * K) * norm;
                        }
                    } break;

                    case Type::Bandpass: {
                        const Sample norm = Sample(1) / (Sample(1) + K / Q + K * K);
                        this->b[0] = K / Q * norm;
                        this->b[1] = Sample(0);
                        this->b[2] = -this->b[0];
                        this->a[0] = Sample(2) * (K * K - Sample(1)) * norm;
                        this->a[1] = (Sample(1) - K / Q + K * K) * norm;
                    } break;

                    case Type::Peak: {
                        if (V >= Sample(1)) {
                            const Sample norm = Sample(1) / (Sample(1) + K / Q + K * K);
                            this->b[0] = (Sample(1) + K / Q * V + K * K) * norm;
                            this->b[1] = Sample(2) * (K * K - Sample(1)) * norm;
                            this->b[2] = (Sample(1) - K / Q * V + K * K) * norm;
                            this->a[0] = this->b[1];
                            this->a[1] = (Sample(1) - K / Q + K * K) * norm;
                        }
                        else {
                            const Sample norm = Sample(1) / (Sample(1) + K / Q / V + K * K);
                            this->b[0] = (Sample(1) + K / Q + K * K) * norm;
                            this->b[1] = Sample(2) * (K * K - Sample(1)) * norm;
                            this->b[2] = (Sample(1) - K / Q + K * K) * norm;
                            this->a[0] = this->b[1];
                            this->a[1] = (Sample(1) - K / Q / V + K * K) * norm;
                        }
                    } break;

                    case Type::Notch: {
                        const Sample norm = Sample(1) / (Sample(1) + K / Q + K * K);
                        this->b[0] = (Sample(1) + K * K) * norm;
                        this->b[1] = Sample(2) * (K * K - Sample(1)) * norm;
                        this->b[2] = this->b[0];
                        this->a[0] = this->b[1];
                        this->a[1] = (Sample(1) - K / Q + K * K) * norm;
                    } break;

                    default: break;
                }
            }

            /** Filter block processor. */
            IDSP_CONSTEXPR_SINCE_CXX14
            void process(const BufferInterface& input, BufferInterface& output)
            {
                this->_process_block(input, output);
            }

            /** Filter sample processor. */
            IDSP_CONSTEXPR_SINCE_CXX14
            Sample process(Sample input)
            {
                return this->_process_sample(input);
            }

        private:
            IDSP_CONSTEXPR_SINCE_CXX14
            Sample _process_sample(Sample x)
            {
                // Feedback coefficients
                this->xState[0] = x + (this->xState[1] * -this->a[1]) + (this->xState[2] * -this->a[2]);
                // Feedfoward coefficients
                const auto out = (this->xState[0] * this->b[0]) + (this->xState[1] * this->b[1]) + (this->xState[2] * this->b[2]);
                // Shift delay blocks
                this->xState[2] = this->xState[1];
                this->xState[1] = this->xState[0];
                return out;
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            void _process_block(const BufferInterface& input, BufferInterface& output)
            {
                const size_t size = idsp::min(input.size(), output.size());
                for (size_t i = 0; i < size; i++)
                {
                    output[i] = this->_process_sample(input[i]);
                }
            }

            Type type;

            std::array<Sample, 3> a;
            std::array<Sample, 3> b;
            std::array<Sample, 3> xState;
    };

    class ToneControl
    {
        public:
            using FilterType = typename BiquadFilter::Type;

            ToneControl(float sample_rate) :
            lowpass{FilterType::Lowpass, (1000.f*(1.f/sample_rate)), 0.3f, 1.f},
            lowshelf{FilterType::Lowshelf, (3000.f*(1.f/sample_rate)), 0.3f, 0.5f},
            highshelf{FilterType::Highshelf, (3000.f*(1.f/sample_rate)), 0.3f, 0.8f},
            highpass{FilterType::Highpass, (8000.f*(1.f/sample_rate)), 0.3f, 1.f},
            tone{0.5}
            {}

            ToneControl(): ToneControl(48000) {}

            IDSP_CONSTEXPR_SINCE_CXX14
            void process(const BufferInterface& input, BufferInterface& output)
            {
                for(size_t i = 0; i < input.size(); i++)
                    output[i] = _process(input[i]);
            }

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            void process_for(const BufferInterface& input, BufferInterface& output)
            {
                for(size_t i = 0; i < N; i++)
                    output[i] = _process(input[i]);
            }

            inline void set_tone(float f) {tone = f;}

            void set_sample_rate(float sample_rate)
            {
                this->lowpass.set_parameters(1000.f*(1.f/sample_rate), 0.3f, 1.f);
                this->lowshelf.set_parameters(3000.f*(1.f/sample_rate), 0.3f, 0.5f);
                this->highshelf.set_parameters(3000.f*(1.f/sample_rate), 0.3f, 0.8f);
                this->highpass.set_parameters(8000.f*(1.f/sample_rate), 0.3f, 1.f);
            }

        private:
            Sample _process(const Sample input)
            {
                float fade_in;
                float fade_out;
                Sample tone_a = input;
                Sample tone_b = input;

                if(tone < 0.2f){
                    fade_in = idsp::rescale(tone, 0.f, 0.2f, 0.f, 1.f);
                    fade_out = 1.f - fade_in;
                    tone_a = (lowpass.process(input) * 6.f);
                    tone_b = (highshelf.process(input) * 2.f);
                }
                else if(tone < 0.4f){
                    fade_in = idsp::rescale(tone, 0.2f, 0.4f, 0.f, 1.f);
                    fade_out = 1.f - fade_in;
                    tone_a = (highshelf.process(input) * 2.f);
                }
                else if(tone < 0.6f){
                    fade_in = idsp::rescale(tone, 0.4f, 0.6f, 0.f, 1.f);
                    fade_out = 1.f - fade_in;
                }
                else if(tone < 0.8f){
                    fade_in = idsp::rescale(tone, 0.6f, 0.8f, 0.f, 1.f);
                    fade_out = 1.f - fade_in;
                    tone_b = (lowshelf.process(input) * 2.f);
                }
                else{
                    fade_in = idsp::rescale(tone, 0.8f, 1.f, 0.f, 1.f);
                    fade_out = 1.f - fade_in;
                    tone_a = (lowshelf.process(input) * 2.f);
                    tone_b = (highpass.process(input));
                }
                return (tone_a * fade_out) + (tone_b * fade_in);
            }

            BiquadFilter lowpass;
            BiquadFilter lowshelf;
            BiquadFilter highshelf;
            BiquadFilter highpass;

            float tone;
    };

} // namespace idsp

#endif