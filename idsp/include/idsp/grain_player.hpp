#ifndef IDSP_GRAIN_PLAYER_H
#define IDSP_GRAIN_PLAYER_H

#include "idsp/constants.hpp"
#include "idsp/lookup.hpp"
#include "idsp/ringbuffer.hpp"
#include "idsp/filter.hpp"
#include "idsp/modulation.hpp"
#include "idsp/delay.hpp"

namespace idsp
{
    template<size_t S>
    class WindowBank
    {
        public:
            WindowBank() :
            cosine{LookupTable<Sample, S>([](Sample p) -> Sample {
                return 0.5 - 0.5 * cosf(2.f * pi * p);
            })},
            triangle{LookupTable<Sample, S>([](Sample p) -> Sample {
                if(p <= 0.5)
                    return (p * 2.f);
                else
                    return (1.f - p) * 2.f;
            })},
            square{LookupTable<Sample, S>([](Sample p) -> Sample {
                if(p < 0.005f)
                    return 0.f;
                else if(p < 0.1f)
                    return p/0.1f;
                else if (p < 0.9f)
                    return 1.f;
                else
                    return 1.f - ((p - 0.9f)* 10.f);
            })},
            sawtooth{LookupTable<Sample, S>([](Sample p) -> Sample {
                 if(p < 0.1f)
                    return p/0.1f;
                else
                    return  1.f - ((p - 0.1)* 1.1f);
            })}
            {}

            inline float get_cosine(float phase){return cosine.read_wrap(phase);}

            inline float get_triangle(float phase){return triangle.read_wrap(phase);}

            inline float get_square(float phase){return square.read_wrap(phase);}

            inline float get_sawtooth(float phase) {return sawtooth.read_wrap(phase);}

            float get_window(float phase, float shape)
            {
                if(shape < 0.3f)
                {
                    float blend = rescale(shape, 0.f, 0.3f, 0.f, 1.f);
                    float sqr = square.read_wrap(phase);
                    float saw = sawtooth.read_wrap(phase);
                    return interpolate_2(blend, sqr, saw);
                }
                else if(shape < 0.6f)
                {
                    float blend = rescale(shape, 0.3f, 0.6f, 0.f, 1.f);
                    float saw = sawtooth.read_wrap(phase);
                    float tri = triangle.read_wrap(phase);
                    return interpolate_2(blend, saw, tri);
                }
                else
                {
                    float blend = rescale(shape, 0.6f, 1.f, 0.f, 1.f);
                    float tri = triangle.read_wrap(phase);
                    float cos = cosine.read_wrap(phase);
                    return interpolate_2(blend, tri, cos);
                }
            }

        private:
            LookupTable<Sample, S> cosine;
            LookupTable<Sample, S> triangle;
            LookupTable<Sample, S> square;
            LookupTable<Sample, S> sawtooth;
    };

    class Grain
    {
        public:
            using FilterType = typename OnepoleFilter::Type;

            static constexpr float min_pitch = 0.25f;
            static constexpr float max_pitch = 4.f;

            struct Parameters
            {
                int position;
                float pitch;
                float length_pot;
                float window_shape;
                size_t channel;
                float volume;
            };

            enum GrainState
            {
                idle,
                active,
                dying
            };

            Grain():
            dc_blocker{FilterType::Highpass}
            {}

            Grain(float sample_rate)  :
            phase{0},
            state{idle},
            sample_rate{sample_rate},
            dc_blocker{FilterType::Highpass, (50*(1.f/sample_rate))}
            {}

            void init(Parameters& paramaters, AudioRingBuffer& audio_buffer)
            {
                grain_parameters = paramaters;
                grain_length = max((static_cast<float>((audio_buffer.get_size()-3) / static_cast<int>(max_pitch)) * grain_parameters.length_pot), 15 * (sample_rate / 1000));
                if(grain_parameters.pitch > 1.f) grain_length *= (1.f / clamp(grain_parameters.pitch, 1.f, 4.f));
                else grain_length *= clamp(grain_parameters.pitch, 0.f, 1.f);
                start_index = (audio_buffer.get_index()) - (static_cast<int>(grain_length) + grain_parameters.position);
                start_index = start_index < 0 ? (start_index + audio_buffer.get_size()) : start_index;
                phase = 0;
                state = active;
            }

            void kill()
            {
                state = idle;
                phase = 0;
            }

            void process(AudioRingBuffer& audio_buffer, WindowBank<128>& windows, Sample& out_left, Sample& out_right)
            {
                float envelope = windows.get_window((phase/grain_length), grain_parameters.window_shape);
                float read_position = static_cast<float>(start_index) + phase;
                Sample out_sample = (audio_buffer.read_at_smooth_safe(read_position) * grain_parameters.volume * envelope);

                phase += grain_parameters.pitch;
                if(phase >= grain_length) this->state = dying;

                out_left += dc_blocker.process(out_sample);

        //        if(grain_parameters.channel == 0) out_left += dc_blocker.process(out_sample);
        //        else out_right += dc_blocker.process(out_sample);
            }


            inline GrainState get_state() {return state;}

        private:
            Parameters grain_parameters;
            float grain_length;
            int start_index;
            float phase;
            GrainState state;
            float sample_rate;
            idsp::OnepoleFilter dc_blocker;
    };

    class GrainPlayer
    {
        public:
            GrainPlayer(AudioRingBuffer& buff) :
            audio_buffer{buff},
            buffer_length_samples{audio_buffer.get_size()},
            max_grain_length{(buffer_length_samples-3) / static_cast<int>(Grain::max_pitch)},
            active_grains{0},
            prev_active_grains{0}
            {}

            void process(BufferInterface& output_left, BufferInterface& output_right)
            {
                const size_t block_size = output_left.size();

                for(size_t i = 0; i < block_size; i++)
                {
                    Sample out_left{0};
                    Sample out_right{0};
                    for(size_t g = 0; g < max_grains; g++)
                    {
                        if(grains[g].get_state() == Grain::active)
                        {
                            grains[g].process(audio_buffer, windows, out_left, out_right);
                        }
                    }

                    output_left[i] =  idsp::tanh_fast(out_left);
                    output_right[i] = idsp::tanh_fast(out_right);
                }

                prev_active_grains = active_grains;

                for(size_t i = 0; i < max_grains; i++)
                {
                    if(grains[i].get_state() == Grain::dying)
                    {
                        grains[i].kill();
                        active_grains--;
                    }
                }
            }

            template<size_t N>
            void process_for(BufferInterface& output_left, BufferInterface& output_right)
            {
                for(size_t i = 0; i < N; i++)
                {
                    Sample out_left{0};
                    Sample out_right{0};
                    for(size_t g = 0; g < max_grains; g++)
                    {
                        if(grains[g].get_state() == Grain::active)
                        {
                            grains[g].process(audio_buffer, windows, out_left, out_right);
                        }
                    }

                    output_left[i] =  idsp::tanh_fast(out_left);
                    output_right[i] = idsp::tanh_fast(out_right);
                }

                prev_active_grains = active_grains;

                for(size_t i = 0; i < max_grains; i++)
                {
                    if(grains[i].get_state() == Grain::dying)
                    {
                        grains[i].kill();
                        active_grains--;
                    }
                }
            }

            void trigger_grain()
            {
                bool grain_triggered = false;
                for(size_t i = 0; i < max_grains; i++)
                {
                    if(grains[i].get_state() == Grain::idle)
                    {
                        active_grains++;
                        player_paramaters.volume = 1.f;
                        player_paramaters.channel = stochastic.coin_toss(50);

                        grains[i].init(player_paramaters, audio_buffer);
                        grain_triggered = true;
                    }
                    if(grain_triggered == true) break;
                }
            }

            inline void set_position(float f) {player_paramaters.position = static_cast<int>(rescale(f, 0.f, 1.f, 1.f, static_cast<float>(buffer_length_samples-max_grain_length)));}

            inline void set_pitch(float f) {player_paramaters.pitch = clamp(f, Grain::min_pitch, Grain::max_pitch);}

            inline void set_length(float f) {player_paramaters.length_pot = f;}

            inline void set_window_shape(float f) {player_paramaters.window_shape = f;}

        private:
            //Granular Constants
            AudioRingBuffer& audio_buffer;
            size_t buffer_length_samples;
            size_t max_grain_length;
            static constexpr size_t max_grains = 16;
            std::array<Grain, max_grains> grains;
            Grain::Parameters player_paramaters;
            unsigned active_grains;
            unsigned prev_active_grains;
            WindowBank<128> windows;
            float feedback;
            Stochastic stochastic;
    };

} // namespace idsp
#endif
