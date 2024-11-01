#ifndef IDSP_RANDOM_H
#define IDSP_RANDOM_H

#include "idsp/oscillator.hpp"

#include <random>

namespace idsp
{
    /** White Noise - uniform distribution */
    class WhiteNoise
    {
        public:

            WhiteNoise()
            {
                randomiser = std::mt19937(random_device());
                distribution = std::uniform_real_distribution<float>(0.f, 1.f);
            }

            inline float positive() {
                return distribution(randomiser);
            }

            inline float bipolar() {
                return (distribution(randomiser) * 2.f) - 1.f;
            }

        private:
            std::random_device random_device;
            std::mt19937 randomiser;
            std::uniform_real_distribution<float> distribution;
    };

    /** Pink Noise - -3db per octave */
    class PinkNoise
    {
        public:
            PinkNoise() = default;

            constexpr
            inline float positive() { return (bipolar() + 1.f) * 0.5f; }

            constexpr
            inline float bipolar()
            {
                int lastFrame = frame;
                frame++;
                if (frame >= (1 << QUALITY))
                    frame = 0;
                int diff = lastFrame ^ frame;

                float pink = 0.f;
                for (size_t i = 0; i < QUALITY; i++) {
                    if (diff & (1 << i)) {
                        values[i] = white_noise.positive() - 0.5f;
                    }
                    pink += values[i];
                }
                return clamp((pink*0.5f), -1.f, 1.f);
            }

        private:
            static constexpr size_t QUALITY = 8;
            int frame = -1;
            std::array<float, QUALITY> values{0};
            WhiteNoise white_noise;
    };

    /** Blue Noise - +3db per octave */
    class BlueNoise
    {
        public:

            BlueNoise() = default;

            constexpr
            inline float positive() { return (bipolar() + 1.f) * 0.5f; }

            constexpr
            inline float bipolar() {
                new_value = pink_noise.bipolar();
                float blue = (new_value - last_value) / 0.705f;
				last_value = new_value;
                return clamp(blue, -1.f, 1.f);
            }

        private:
            PinkNoise pink_noise;
            float new_value;
            float last_value;
    };

    class NoiseSource
    {
        public:

            enum class Colour
            {
                white,
                pink,
                blue
            };

            NoiseSource() :
            noise_colour{Colour::white}
            {}

            constexpr
            inline float positive()
            {
                switch (noise_colour)
                {
                    case Colour::white:
                        return white_noise.positive();
                    break;

                    case Colour::pink:
                        return pink_noise.positive();
                    break;

                    case Colour::blue:
                        return blue_noise.positive();
                    break;

                    default:
                        return white_noise.positive();
                    break;
                }
            }

            constexpr
            inline float bipolar()
            {
                switch (noise_colour)
                {
                    case Colour::white:
                        return white_noise.bipolar();
                    break;

                    case Colour::pink:
                        return pink_noise.bipolar();
                    break;

                    case Colour::blue:
                        return blue_noise.bipolar();
                    break;

                    default:
                        return white_noise.bipolar();
                    break;
                }
            }

            constexpr
            inline void set_noise_colour(Colour colour) {noise_colour = colour;}

        private:
            WhiteNoise white_noise;
            PinkNoise pink_noise;
            BlueNoise blue_noise;
            Colour noise_colour;
    };

    class Stochastic
    {
        public:
            Stochastic()
            {}

            constexpr
            inline bool coin_toss(float odds)
            {
                if((noise.positive()*100.f) < odds) return true;
                else return false;
            }

            constexpr
            inline int get_range(int min, int max)
            {
                const float result = rescale(noise.bipolar(), -1.f, 1.f, static_cast<float>(min), static_cast<float>(max));
                return static_cast<int>(result);
            }

            constexpr
            inline float get_range(float min, float max)
            {
                const float result = rescale(noise.bipolar(), -1.f, 1.f, min, max);
                return result;

            }

            constexpr
            inline float get_positive() {return noise.positive();}

            constexpr
            inline float get_bipolar() {return noise.bipolar();}

            constexpr
            inline void set_noise_colour(NoiseSource::Colour colour) {noise.set_noise_colour(colour);}

        private:
            NoiseSource noise;

    };

class FluctuatingRandom
    {
        public:
            FluctuatingRandom() :
            lfo(idsp::Waveform::square, false),
            density{100.f},
            previous_sample{0},
            new_sample{0},
            smooth_amount{0},
            range{max_range},
            trigger{false},
            bipolar{true},
            maximum_density{127.f}
            {}

            template<size_t N>
            inline void process_for(BufferInterface& output)
            {
                for(size_t i = 0; i < N; i++)
                {
                    output[i] = process();
                }
            }

            inline float process()
            {
                float pulse = lfo.process();
                if (pulse > 0.5f && !trigger)
                {
                    if(probability.coin_toss(density))
                    {
                        if(bipolar) new_sample = _quantise(noise.bipolar());
                        else _quantise(noise.positive());
                    }
                    trigger = true;
                }
                else if (pulse < 0.5f) trigger = false;

                return previous_sample += ((new_sample - previous_sample) / max(smooth_amount, 1.f));
            }

            template<size_t N>
            inline void process_for(bool clock_in, BufferInterface& output)
            {
                for(size_t i = 0; i < N; i++)
                {
                    output[i] = process(clock_in);
                }
            }

            inline float process(bool clock_in)
            {
               if(clock_in && probability.coin_toss(density))
                {
                    if (bipolar) new_sample = _quantise(noise.bipolar());
                    else new_sample = _quantise(noise.positive());
                }
                return previous_sample += ((new_sample - previous_sample) / max(smooth_amount, 1.f));
            }

            template<size_t N>
            inline void process_retriggable_for(bool trigger, BufferInterface& output)
            {
                for(size_t i = 0; i < N; i++)
                {
                    output[i] = process_retriggable(trigger);
                }
            }

            inline float process_retriggable(bool retrig)
            {
                float pulse = lfo.process();
                if (retrig || (pulse > 0.5f && !trigger))
                {
                    if(probability.coin_toss(density))
                    {
                        if (bipolar) new_sample = _quantise(noise.bipolar());
                        else _quantise(noise.positive());
                    }
                    trigger = true;
                }
                else if (pulse < 0.5f) trigger = false;

                return previous_sample += ((new_sample - previous_sample) / max(smooth_amount, 1.f));
            }

            constexpr
            inline void set_rate(float f) {lfo.set_rate(f);}

            constexpr
            inline void set_noise_colour(NoiseSource::Colour colour) { noise.set_noise_colour(colour); }

            constexpr
            inline void set_density(float f) {density = f * maximum_density;}

            constexpr
            inline void set_maximum_density(float f) {maximum_density = f;}

            constexpr
            inline void set_range(float f) {range = max(static_cast<unsigned>(f * max_range), 1u);}

            constexpr
            inline void set_smooth_amount(float f) { smooth_amount = f * max_slew; }

            constexpr
            inline bool pulse_out() {return trigger;}

            constexpr
            inline void set_bipolar(bool polarity) { bipolar = polarity; }

        private:
            float _quantise(float f)
            {
                unsigned unquantised = static_cast<unsigned>(((f + 1.f) * 0.5f) * max_range);
                unsigned step = max_range / range;
                return ((static_cast<float>((unquantised / step) * step) / max_range) * 2.f) -1.f;
            }

            Stochastic probability;
            NoiseSource noise;
            WavetableOscillator<128> lfo;
            float density;
            float previous_sample;
            float new_sample;
            float smooth_amount;
            unsigned range;
            bool trigger;
            bool bipolar;
            float maximum_density;
            static constexpr unsigned max_range {100};
            static constexpr float max_slew {48000};

    };
} // namespace idsp


#endif
