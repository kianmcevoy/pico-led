#ifndef MOD_FX_HPP
#define MOD_FX_HPP

#include "idsp/reverb_toolkit.hpp"
#include "idsp/buffer_types.hpp"
#include "idsp/modulation.hpp"
#include "idsp/filter.hpp"

namespace idsp
{
    class Chorus
    {
        public:
            Chorus() = default;

            Chorus(float samplerate):
            modulation_depth{1.f},
            modulation_amount{0.f},
            sample_rate{samplerate},
            lfo_a(idsp::Waveform::triangle, false),
            lfo_b(idsp::Waveform::triangle, false)
            {
                lfo_a.set_rate(1.f/sample_rate);
                lfo_b.set_rate(0.99f/sample_rate);
            }
            void process(const BufferInterface& input, PolyBufferInterface& output)
            {
                for(size_t i = 0; i < input.size(); i++)
                    _process(input[i], output[0][i], output[1][i]);

            }

            template<size_t N>
            void process_for(const BufferInterface& input, PolyBufferInterface& output)
            {
                for(size_t i = 0; i < N; i++)
                    _process(input[i], output[0][i], output[1][i]);
            }

            inline void set_modulation_amount(float f) {modulation_amount = clamp(f, 0.f, 1.f);}

            inline void set_modulation_rate(float f)
            {
                lfo_a.set_rate((f * f * 0.5f) / sample_rate);
                lfo_b.set_rate((f * f * 0.49f) / sample_rate);
            }


            void _process(const Sample input, Sample& out_left, Sample& out_right)
             {
                float blend = idsp::clamp((modulation_amount * 1.5f), 0.f, 1.f);
                modulation_depth = modulation_amount < 0.25f ? 0 : idsp::rescale(modulation_amount, 0.25f, 1.f, 0.f, 0.5f);

                float mod_a = lfo_a.process();
                float mod_b = lfo_b.process();
                Sample chorus = delay->read_offset_smooth_wrap((mod_a * (max_depth * modulation_depth)) + 48);
                Sample chorus_b = delay->read_offset_smooth_wrap((mod_b * (max_depth * modulation_depth)) + 48);

                delay->write(input);

                out_left = interpolate_2(blend, input, (chorus * 0.5f) + (input * 0.5f));
                out_right = interpolate_2(blend, input, (chorus_b * 0.5f) + (input * 0.5f));
            }

            private:

            float modulation_depth;

            float modulation_amount;

            float sample_rate;

            static constexpr size_t max_depth = 4800;

            Delay<SampleBufferStatic<5280>> delay;

            WavetableOscillator<128> lfo_a;
            WavetableOscillator<128> lfo_b;
    };

    class Tremolo_OnePot
    {
        public:
            Tremolo_OnePot() = default;

            Tremolo_OnePot(float samplerate) :
            depth{0},
            sample_rate{samplerate},
            lfo_left{Waveform::sine, true},
            lfo_right{Waveform::sine, true}
            {}

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

            void set_modulation_amount(float f)
            {
                depth = f;
                float offset = depth < 0.5f ? 0 : idsp::rescale(depth, 0.5f, 1.f, 0.f, 1.f);
                lfo_left.set_rate(((rate_left-(depth/10.f)) + (offset*8.f)) * (1.f/sample_rate));
                lfo_right.set_rate(((rate_right+(depth/10.f)) + (offset*8.f)) * (1.f/sample_rate));
            }

        private:
            Sample _process(const Sample input)
            {
                float blend = idsp::clamp((depth * 4.f), 0.f, 1.f);
                Sample out = interpolate_2(min((depth * 2.f), 1.f), input, input * (lfo_left.process() * depth));
                Sample out_b = interpolate_2(min((depth * 2.f), 1.f), input, input * (lfo_right.process() * depth));

                return interpolate_2(blend, input, ((out + out_b) * 0.5f));
            }

            float depth;
            float sample_rate;
            static constexpr float rate_left {0.97129f};
            static constexpr float rate_right {0.81246f};
            WavetableOscillator<128> lfo_left;
            WavetableOscillator<128> lfo_right;
    };

    class Phaser_6_Pole
    {
        public:
            using FilterType = typename idsp::OnepoleFilter::Type;

            Phaser_6_Pole(float samplerate) :
            apf1{FilterType::Allpass},
            apf2{FilterType::Allpass},
            apf3{FilterType::Allpass},
            apf4{FilterType::Allpass},
            apf5{FilterType::Allpass},
            apf6{FilterType::Allpass},
            depth{0},
            feedback{0},
            feedback_amount{0},
            blend{0},
            sample_rate{samplerate},
            lfo(Waveform::triangle, false)
            {}

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

            constexpr
            inline void set_modulation_depth(float f) {depth = f;}

            constexpr
            inline void set_modulation_rate(float f) {lfo.set_rate(f*(1.f/sample_rate));}

            constexpr
            inline void set_modulation_phase(float f) {lfo.set_phase_offset(f*0.5f);}

            constexpr
            inline void set_feedback(float f) {feedback_amount = f;}

            constexpr
            inline void set_blend(float f) {blend = f;}

        private:
            Sample _process(const Sample input)
            {
                const float mod = lfo.process() * depth;

                const float freq_1 = rescale(mod, 0.f, 1.f, apf1_min_freq, apf1_max_freq);
                const float freq_2 = rescale(mod, 0.f, 1.f, apf2_min_freq, apf2_max_freq);
                const float freq_3 = rescale(mod, 0.f, 1.f, apf3_min_freq, apf3_max_freq);
                const float freq_4 = rescale(mod, 0.f, 1.f, apf4_min_freq, apf4_max_freq);
                const float freq_5 = rescale(mod, 0.f, 1.f, apf5_min_freq, apf5_max_freq);
                const float freq_6 = rescale(mod, 0.f, 1.f, apf6_min_freq, apf6_max_freq);

                apf1.set_cutoff(freq_1 / sample_rate);
                apf2.set_cutoff(freq_2 / sample_rate);
                apf3.set_cutoff(freq_3 / sample_rate);
                apf4.set_cutoff(freq_4 / sample_rate);
                apf5.set_cutoff(freq_5 / sample_rate);
                apf6.set_cutoff(freq_6 / sample_rate);

                Sample pole_1 = apf1.process(tanh_fast(input + (feedback * feedback_amount)));
                Sample pole_2 = apf2.process(pole_1);
                Sample pole_3 = apf3.process(pole_2);
                Sample pole_4 = apf4.process(pole_3);
                Sample pole_5 = apf5.process(pole_4);
                Sample pole_6 = apf6.process(pole_5);

                feedback = pole_6;

                return interpolate_2(blend, (input*0.707f), (pole_6*0.707f));
            }

            OnepoleFilter apf1;
            OnepoleFilter apf2;
            OnepoleFilter apf3;
            OnepoleFilter apf4;
            OnepoleFilter apf5;
            OnepoleFilter apf6;
            float depth;
            Sample feedback;
            float feedback_amount;
            float blend;
            float sample_rate;
            WavetableOscillator<128> lfo;
            static constexpr float apf1_min_freq = 32.f;
            static constexpr float apf2_min_freq = 68.f;
            static constexpr float apf3_min_freq = 96.f;
            static constexpr float apf4_min_freq = 212.f;
            static constexpr float apf5_min_freq = 320.f;
            static constexpr float apf6_min_freq = 636.f;

            static constexpr float apf1_max_freq = 1500.f;
            static constexpr float apf2_max_freq = 3400.f;
            static constexpr float apf3_max_freq = 4800.f;
            static constexpr float apf4_max_freq = 10000.f;
            static constexpr float apf5_max_freq = 16000.f;
            static constexpr float apf6_max_freq = 20480.f;
    };

    class WowFlutter
    {
        public:
            WowFlutter() = default;

            WowFlutter(float samplerate) :
            wow_depth{1400},
            flutter_depth{50},
            wow_rate{0.1},
            flutter_rate{1.7},
            modulation_amount{0},
            sample_rate{samplerate},
            wow(Waveform::sine, true),
            flutter(Waveform::sine, true)
            {
                wow.set_rate(0.1f*(1.f/sample_rate));
                flutter.set_rate(1.7f*(1.f/sample_rate));
            }

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

            inline void set_modulation_amount(float f) {modulation_amount = f;}

            inline void set_wow_depth(float f) {wow_depth = f;}

            inline void set_flutter_depth(float f) {flutter_depth = f;}

            inline void set_wow_rate(float f) {wow.set_rate(f*(1.f/sample_rate));}

            inline void set_flutter_rate(float f) {flutter.set_rate(f*(1.f/sample_rate));}

        private:
            Sample _process(const Sample input)
             {
                const float mod_offset = wow_depth + flutter_depth;
                const Sample mod_value = ((wow.process() * wow_depth) + (flutter.process() * flutter_depth)) * modulation_amount;
                Sample mod = delay->read_offset_smooth_wrap(mod_offset + mod_value);
                Sample output = interpolate_2((modulation_amount/2.f), input, mod);
                delay->write(input);
                return output;
            }
            float wow_depth;
            float flutter_depth;
            float wow_rate;
            float flutter_rate;
            float modulation_amount;
            float sample_rate;

            Delay<SampleBufferStatic<3000>> delay;
            WavetableOscillator<128> wow;
            WavetableOscillator<128> flutter;
    };

}//namespace idsp

#endif