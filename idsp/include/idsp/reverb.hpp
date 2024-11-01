#ifndef IDSP_REVERBS
#define IDSP_REVERBS

#include "idsp/reverb_toolkit.hpp"
#include "idsp/delay.hpp"
#include "idsp/buffer_types.hpp"
#include "idsp/filter.hpp"
#include "idsp/constants.hpp"



namespace idsp
{
    template<size_t SAMPLE_RATE>
    class SmallRoom
    {
        using FilterType = typename idsp::OnepoleFilter::Type;
        public:
            SmallRoom()   :
            dampening_l{FilterType::Lowpass, (4500*(1.f/SAMPLE_RATE))},
            feedback_l{0},
            dampening_r{FilterType::Lowpass, (4500*(1.f/SAMPLE_RATE))},
            feedback_r{0},
            gain{0}
            {
                double_nested_allpass_l->set_gain(0.6, 0.4, 0.8);
                nested_allpass_l->set_gain(0.4, 0.1);
                double_nested_allpass_r->set_gain(0.6, 0.4, 0.8);
                nested_allpass_r->set_gain(0.4, 0.1);
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            /**assumes left/right/in/out buffers are all of equal length*/
            void process(const PolyBufferInterface& input, PolyBufferInterface& output)
            {
                for(size_t i = 0; i < input[0].size(); i++)
                    _process(input[0][i], input[1][i], output[0][i], output[1][i]);
            }

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            void process_for(const PolyBufferInterface& input, PolyBufferInterface& output)
            {
                for(size_t i = 0; i < N; i++)
                    _process(input[0][i], input[1][i], output[0][i], output[1][i]);
            }

            constexpr
            inline void set_gain(Sample g) {gain = g;}

            inline void set_dampening(float f) {dampen = f;}

            inline void set_cutoff(float f)
            {
                dampening_l.set_cutoff(f);
                dampening_r.set_cutoff(f);
            }

        private:
            void _process(const Sample in_l, const Sample in_r, Sample& out_l, Sample& out_r)
            {
                // apply dampening to feedback tap
                Sample damp_l = interpolate_2(dampen, feedback_r, dampening_l.process(feedback_r));
                Sample damp_r = interpolate_2(dampen, feedback_l, dampening_r.process(feedback_l));
                // calculate feedforward tap
                Sample feedforward_l = double_nested_allpass_l->process(delay_l->read());
                Sample feedforward_r = double_nested_allpass_r->process(delay_r->read());
                // sample new input + varaible amount of feedback
                delay_l->write(in_l +  (damp_l * gain));
                delay_r->write(in_r +  (damp_r * gain));
                // calculate feedback tap
                feedback_l = nested_allpass_l->process(feedforward_l);
                feedback_r = nested_allpass_r->process(feedforward_r);
                // output a mix of feedforward and feedback tap
                out_l = (feedback_l * 0.2f) + (feedforward_l * 0.6f);
                out_r = (feedback_r * 0.2f) + (feedforward_r * 0.6f);
            }

            Delay<SampleBufferStatic<(24*(SAMPLE_RATE/1000))>>  delay_l;
            DoubleNestedAllpass <SampleBufferStatic<static_cast<size_t>(8.3f * static_cast<float>((SAMPLE_RATE/1000)))>,
                                SampleBufferStatic<(22 * (SAMPLE_RATE/1000))>,
                                SampleBufferStatic<(35 * (SAMPLE_RATE/1000))>> double_nested_allpass_l;
            NestedAllpass<SampleBufferStatic<(30 * (SAMPLE_RATE/1000))>, SampleBufferStatic<(66 * (SAMPLE_RATE/1000))>> nested_allpass_l;
            OnepoleFilter dampening_l;
            Sample feedback_l;

            Delay<SampleBufferStatic<(25*(SAMPLE_RATE/1000))>>  delay_r;
            DoubleNestedAllpass <SampleBufferStatic<static_cast<size_t>(8.f * static_cast<float>((SAMPLE_RATE/1000)))>,
                                SampleBufferStatic<(23 * (SAMPLE_RATE/1000))>,
                                SampleBufferStatic<(34 * (SAMPLE_RATE/1000))>> double_nested_allpass_r;
            NestedAllpass<SampleBufferStatic<(31 * (SAMPLE_RATE/1000))>, SampleBufferStatic<(65 * (SAMPLE_RATE/1000))>> nested_allpass_r;
            OnepoleFilter dampening_r;
            Sample feedback_r;

            Sample gain;
            float dampen;
    };

    template<size_t SAMPLE_RATE>
    class MediumRoom
    {
        using FilterType = typename idsp::OnepoleFilter::Type;
        public:
            MediumRoom()  :
            dampening_l{FilterType::Lowpass, (2500*(1.f/SAMPLE_RATE))},
            feedback_l{0},
            dampening_r{FilterType::Lowpass, (2500*(1.f/SAMPLE_RATE))},
            feedback_r{0}
            {
                allpass_l->set_gain(0.5f);
                nested_allpass_l->set_gain(0.6, 0.3);
                allpass_r->set_gain(0.5f);
                nested_allpass_r->set_gain(0.6, 0.3);
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            /**assumes left/right/in/out buffers are all of equal length*/
            void process(const PolyBufferInterface& input, PolyBufferInterface& output)
            {
                for(size_t i = 0; i < input[0].size(); i++)
                    _process(input[0][i], input[1][i], output[0][i], output[1][i]);
            }

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            void process_for(const PolyBufferInterface& input, PolyBufferInterface& output)
            {
                for(size_t i = 0; i < N; i++)
                    _process(input[0][i], input[1][i], output[0][i], output[1][i]);
            }

            constexpr
            inline void set_gain(Sample g) {gain = g;}

            inline void set_dampening(float f) {dampen = f;}

            inline void set_cutoff(float f)
            {
                dampening_l.set_cutoff(f);
                dampening_r.set_cutoff(f);
            }

        private:
            void _process(const Sample in_l, const Sample in_r, Sample& out_l, Sample& out_r)
             {
                Sample damp_l = interpolate_2(dampen, delay3_l->read(), dampening_l.process(delay3_l->read()));
                Sample damp_r = interpolate_2(dampen, delay3_r->read(), dampening_r.process(delay3_r->read()));

                Sample feedforward0_l = double_nested_allpass_l->process((in_l + (damp_l * gain)));
                Sample feedforward0_r = double_nested_allpass_r->process((in_r + (damp_r * gain)));

                Sample feedforward1_l = allpass_l->process(delay0_l->read());
                Sample feedforward1_r = allpass_r->process(delay0_r->read());

                delay0_l->write(feedforward0_l);
                delay0_r->write(feedforward0_r);

                Sample del1_out_l =  delay1_l->read();
                Sample del1_out_r =  delay1_r->read();
                delay1_l->write(feedforward1_l);
                delay1_r->write(feedforward1_r);
                feedforward1_l = del1_out_l;
                feedforward1_r = del1_out_r;

                feedback_r = nested_allpass_l->process(((delay2_l->read() * gain) + in_l));
                feedback_l = nested_allpass_r->process(((delay2_r->read() * gain) + in_r));

                delay2_l->write(feedforward1_l);
                delay2_r->write(feedforward1_r);

                delay3_l->write(feedback_l);
                delay3_r->write(feedback_r);

                out_l = (feedforward0_l * 0.34f) + (feedforward1_l * 0.14f) + (feedback_l * 0.14f);
                out_r = (feedforward0_r * 0.34f) + (feedforward1_r * 0.14f) + (feedback_r * 0.14f);
            }


            DoubleNestedAllpass <SampleBufferStatic<22 *(SAMPLE_RATE/1000)>,
                                SampleBufferStatic<static_cast<size_t>(8.3f * static_cast<float>((SAMPLE_RATE/1000)))>,
                                SampleBufferStatic<(35*(SAMPLE_RATE/1000))>> double_nested_allpass_l;
            Delay<SampleBufferStatic<(5*(SAMPLE_RATE/1000))>>  delay0_l;
            Allpass<SampleBufferStatic<(30*(SAMPLE_RATE/1000))>> allpass_l;
            Delay<SampleBufferStatic<(67*(SAMPLE_RATE/1000))>>  delay1_l;
            Delay<SampleBufferStatic<(15*(SAMPLE_RATE/1000))>>  delay2_l;
            NestedAllpass<SampleBufferStatic<(10 * (SAMPLE_RATE/1000))>, SampleBufferStatic<(39 * (SAMPLE_RATE/1000))>> nested_allpass_l;
            Delay<SampleBufferStatic<(108*(SAMPLE_RATE/1000))>>  delay3_l;
            OnepoleFilter dampening_l;
            Sample feedback_l;

            DoubleNestedAllpass <SampleBufferStatic<21 *(SAMPLE_RATE/1000)>,
                                SampleBufferStatic<static_cast<size_t>(8.4f * static_cast<float>((SAMPLE_RATE/1000)))>,
                                SampleBufferStatic<(34*(SAMPLE_RATE/1000))>> double_nested_allpass_r;
            Delay<SampleBufferStatic<(6*(SAMPLE_RATE/1000))>>  delay0_r;
            Allpass<SampleBufferStatic<(29*(SAMPLE_RATE/1000))>> allpass_r;
            Delay<SampleBufferStatic<(68*(SAMPLE_RATE/1000))>>  delay1_r;
            Delay<SampleBufferStatic<(14*(SAMPLE_RATE/1000))>>  delay2_r;
            NestedAllpass<SampleBufferStatic<(11 * (SAMPLE_RATE/1000))>, SampleBufferStatic<(38 * (SAMPLE_RATE/1000))>> nested_allpass_r;
            Delay<SampleBufferStatic<(107*(SAMPLE_RATE/1000))>>  delay3_r;
            OnepoleFilter dampening_r;
            Sample feedback_r;

            Sample gain;
            float dampen;
    };

    template<size_t SAMPLE_RATE>
    class LargeRoom
    {
        using FilterType = typename idsp::OnepoleFilter::Type;
        public:
            LargeRoom()  :
            dampening_l{FilterType::Lowpass, (2600*(1.f/SAMPLE_RATE))},
            feedback_l{0},
            dampening_r{FilterType::Lowpass, (2600*(1.f/SAMPLE_RATE))},
            feedback_r{0}
            {
                allpass_l->set_gain(0.3f);
                allpass1_l->set_gain(0.3f);
                nested_allpass_l->set_gain(0.25, 0.5);
                double_nested_allpass_l->set_gain(0.25, 0.25, 0.5);

                allpass_r->set_gain(0.3f);
                allpass1_r->set_gain(0.3f);
                nested_allpass_r->set_gain(0.25, 0.5);
                double_nested_allpass_r->set_gain(0.25, 0.25, 0.5);
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            /**assumes left/right/in/out buffers are all of equal length*/
            void process(const PolyBufferInterface& input, PolyBufferInterface& output)
            {
                for(size_t i = 0; i < input[0].size(); i++)
                    _process(input[0][i], input[1][i], output[0][i], output[1][i]);
            }

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            void process_for(const PolyBufferInterface& input, PolyBufferInterface& output)
            {
                for(size_t i = 0; i < N; i++)
                    _process(input[0][i], input[1][i], output[0][i], output[1][i]);
            }

            constexpr
            inline void set_gain(Sample g) {gain = g;}

            inline void set_dampening(float f) {dampen = f;}

            inline void set_cutoff(float f)
            {
                dampening_l.set_cutoff(f);
                dampening_r.set_cutoff(f);
            }

        private:
            void _process(const Sample in_l, const Sample in_r, Sample& out_l, Sample& out_r)
            {
                feedback_r = double_nested_allpass_l->process(delay3_l->read());
                feedback_l = double_nested_allpass_r->process(delay3_r->read());

                Sample dampened_feedback_l = interpolate_2(dampen, feedback_l, dampening_l.process(feedback_l)) * gain;
                Sample dampened_feedback_r = interpolate_2(dampen, feedback_r, dampening_r.process(feedback_r)) * gain;

                Sample feedforward_1_l = delay_l->read();
                Sample feedforward_1_r = delay_r->read();
                Sample ap_out_l = allpass_l->process(in_l + dampened_feedback_l);
                Sample ap_out_r = allpass_r->process(in_r + dampened_feedback_r);
                delay_l->write(allpass1_l->process(ap_out_l));
                delay_r->write(allpass1_r->process(ap_out_r));

                Sample nested_ap_out_l = nested_allpass_l->process(delay1_l->read());
                Sample nested_ap_out_r = nested_allpass_r->process(delay1_r->read());
                delay1_l->write(feedforward_1_l);
                delay1_r->write(feedforward_1_r);

                Sample feedforward_2_l = delay2_l->read();
                Sample feedforward_2_r = delay2_r->read();
                delay2_l->write(nested_ap_out_l);
                delay2_r->write(nested_ap_out_r);

                delay3_l->write(feedforward_2_l);
                delay3_r->write(feedforward_2_r);

                out_l = (feedforward_1_l * 0.34f) + (feedforward_2_l * 0.14f) + (feedback_l * 0.14f);
                out_r = (feedforward_1_r * 0.34f) + (feedforward_2_r * 0.14f) + (feedback_r * 0.14f);
            }

            Allpass<SampleBufferStatic<(8*(SAMPLE_RATE/1000))>> allpass_l;
            Allpass<SampleBufferStatic<(12*(SAMPLE_RATE/1000))>> allpass1_l;
            Delay<SampleBufferStatic<(4*(SAMPLE_RATE/1000))>>  delay_l;
            Delay<SampleBufferStatic<(17*(SAMPLE_RATE/1000))>>  delay1_l;
            NestedAllpass<SampleBufferStatic<(62 * (SAMPLE_RATE/1000))>,
                        SampleBufferStatic<(87 * (SAMPLE_RATE/1000))>> nested_allpass_l;
            Delay<SampleBufferStatic<(31*(SAMPLE_RATE/1000))>>  delay2_l;
            Delay<SampleBufferStatic<(3*(SAMPLE_RATE/1000))>>  delay3_l;
            DoubleNestedAllpass <SampleBufferStatic<30 *(SAMPLE_RATE/1000)>,
                                SampleBufferStatic<76 * (SAMPLE_RATE/1000)>,
                                SampleBufferStatic<120*(SAMPLE_RATE/1000)>> double_nested_allpass_l;
            OnepoleFilter dampening_l;
            Sample feedback_l;

            Allpass<SampleBufferStatic<(9*(SAMPLE_RATE/1000))>> allpass_r;
            Allpass<SampleBufferStatic<(11*(SAMPLE_RATE/1000))>> allpass1_r;
            Delay<SampleBufferStatic<(5*(SAMPLE_RATE/1000))>>  delay_r;
            Delay<SampleBufferStatic<(16*(SAMPLE_RATE/1000))>>  delay1_r;
            NestedAllpass<SampleBufferStatic<(61 * (SAMPLE_RATE/1000))>,
                        SampleBufferStatic<(86 * (SAMPLE_RATE/1000))>> nested_allpass_r;
            Delay<SampleBufferStatic<(32*(SAMPLE_RATE/1000))>>  delay2_r;
            Delay<SampleBufferStatic<(2*(SAMPLE_RATE/1000))>>  delay3_r;
            DoubleNestedAllpass <SampleBufferStatic<31 *(SAMPLE_RATE/1000)>,
                                SampleBufferStatic<75 * (SAMPLE_RATE/1000)>,
                                SampleBufferStatic<121*(SAMPLE_RATE/1000)>> double_nested_allpass_r;
            OnepoleFilter dampening_r;
            Sample feedback_r;

            Sample gain;
            float dampen;
    };

    template<size_t SAMPLE_RATE>
    class DattorroPlate
    {
        using FilterType = typename idsp::OnepoleFilter::Type;
        public:
            DattorroPlate():
            diffusion{FilterType::Lowpass, (9600*(1.f/SAMPLE_RATE))},
            dampening_A{FilterType::Lowpass, (6500*(1.f/SAMPLE_RATE))},
            dampening_B{FilterType::Lowpass, (6500*(1.f/SAMPLE_RATE))}
            {
                allpass1->set_gain(0.75f);
                allpass2->set_gain(0.75f);
                allpass3->set_gain(0.625f);
                allpass4->set_gain(0.625f);
                ap_mod_A->set_gain(0.35f);
                ap_mod_A->set_modulation_rate(0.7f*(1.f/SAMPLE_RATE));
                ap_mod_A->set_sample_depth(50);
                allpass_A->set_gain(0.5);
                ap_mod_B->set_gain(0.475f);
                ap_mod_B->set_modulation_rate(0.6f*(1.f/SAMPLE_RATE));
                ap_mod_B->set_sample_depth(50);
                allpass_B->set_gain(0.5f);
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            void process(const BufferInterface& input, PolyBufferInterface& output)
            {
                for(size_t i = 0; i < input.size(); i++)
                    _process(input[i], output[0][i], output[1][i]);
            }

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            void process_for(const BufferInterface& input, PolyBufferInterface& output)
            {
                for(size_t i = 0; i < N; i++)
                    _process(input[i], output[0][i], output[1][i]);
            }

            constexpr
            inline void set_gain(Sample g) {gain = g;}


        private:
            void _process(const Sample input, Sample& output_left, Sample& output_right)
            {
                //prepare outputs
                Sample tap1 = delay1_A->read_offset(static_cast<size_t>(8.932*(SAMPLE_RATE/1000))) * 0.3;
                Sample tap2 = delay1_A->read_offset(static_cast<size_t>(99.795*(SAMPLE_RATE/1000))) * 0.3;
                Sample tap3 = allpass_A->read_offset(static_cast<size_t>(54.195*(SAMPLE_RATE/1000))) * 0.3;
                Sample tap4 = delay2_A->read_offset(static_cast<size_t>(66.984*(SAMPLE_RATE/1000))) * 0.3;
                Sample tap5 = delay1_B->read_offset(static_cast<size_t>(66.780*(SAMPLE_RATE/1000))) * 0.3;
                Sample tap6 = allpass_B->read_offset(static_cast<size_t>(6.2811*(SAMPLE_RATE/1000))) * 0.3;
                Sample tap7 = delay2_B->read_offset(static_cast<size_t>(35.782*(SAMPLE_RATE/1000))) * 0.3;

                Sample tap8 = delay1_B->read_offset(static_cast<size_t>(11.836*(SAMPLE_RATE/1000))) * 0.3;
                Sample tap9 = delay1_B->read_offset(static_cast<size_t>(121.723*(SAMPLE_RATE/1000))) * 0.3;
                Sample tap10 = allpass_B->read_offset(static_cast<size_t>(41.201*(SAMPLE_RATE/1000))) * 0.3;
                Sample tap11 = delay2_B->read_offset(static_cast<size_t>(89.705*(SAMPLE_RATE/1000))) * 0.3;
                Sample tap12 = delay1_A->read_offset(static_cast<size_t>(70.839*(SAMPLE_RATE/1000))) * 0.3;
                Sample tap13 = allpass_A->read_offset(static_cast<size_t>(11.2471*(SAMPLE_RATE/1000))) * 0.3;
                Sample tap14 = delay2_A->read_offset(static_cast<size_t>(4.058*(SAMPLE_RATE/1000))) * 0.3;

                Sample diffusion_out = diffusion.process(pre_delay->read());
                pre_delay->write(input);

                Sample ap_out1 = allpass1->process(diffusion_out);
                Sample ap_out2 = allpass2->process(ap_out1);
                Sample ap_out3 = allpass3->process(ap_out2);
                Sample ap_out4 = allpass4->process(ap_out3);


                feedbackA = delay2_A->read() * gain;
                feedbackB = delay2_B->read() * gain;

                Sample damp_out_A = dampening_A.process(delay1_A->read());
                Sample damp_out_B = dampening_B.process(delay1_B->read());
                delay2_A->write(allpass_A->process(damp_out_A));
                delay2_B->write(allpass_B->process(damp_out_B));

                delay1_A->write(ap_mod_A->process(ap_out4 + feedbackB));
                delay1_B->write(ap_mod_B->process(ap_out4 + feedbackA));

                output_left = tap1 + tap2 - tap3 + tap4 - tap5 - tap6 - tap7;
                output_right = tap8 + tap9 - tap10 + tap11 - tap12 - tap13 - tap14;
            }

            Delay<SampleBufferStatic<static_cast<size_t>(20.f*(static_cast<float>(SAMPLE_RATE)/1000.f))>> pre_delay;
            OnepoleFilter diffusion;
            Allpass<SampleBufferStatic<static_cast<size_t>(4.76f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> allpass1;
            Allpass<SampleBufferStatic<static_cast<size_t>(3.58f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> allpass2;
            Allpass<SampleBufferStatic<static_cast<size_t>(12.721f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> allpass3;
            Allpass<SampleBufferStatic<static_cast<size_t>(9.297f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> allpass4;

            ModulatedAllpass<SampleBufferStatic<static_cast<size_t>(12.f*(static_cast<float>(SAMPLE_RATE)/1000.f))>> ap_mod_A;
            Delay<SampleBufferStatic<static_cast<size_t>(141.51f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> delay1_A;
            OnepoleFilter dampening_A;
            Allpass<SampleBufferStatic<static_cast<size_t>(60.40f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> allpass_A;
            Delay<SampleBufferStatic<static_cast<size_t>(105.238f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> delay2_A;

            ModulatedAllpass<SampleBufferStatic<static_cast<size_t>(8.f*(static_cast<float>(SAMPLE_RATE)/1000.f))>> ap_mod_B;
            Delay<SampleBufferStatic<static_cast<size_t>(149.433f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> delay1_B;
            OnepoleFilter dampening_B;
            Allpass<SampleBufferStatic<static_cast<size_t>(89.32f*(static_cast<float>(SAMPLE_RATE)/1000.f))>> allpass_B;
            Delay<SampleBufferStatic<static_cast<size_t>(124.829f*(static_cast<float>(SAMPLE_RATE)/1000.f))>> delay2_B;

            Sample gain;
            Sample feedbackA;
            Sample feedbackB;
    };

    template<size_t SAMPLE_RATE>
    class GreisengerClouds
    {
        using FilterType = typename idsp::OnepoleFilter::Type;
        public:
            GreisengerClouds() :
            dampening_a{FilterType::Lowpass, (22000*(1.f/SAMPLE_RATE))},
            dampening_b{FilterType::Lowpass, (22000*(1.f/SAMPLE_RATE))}
            {
                allpass1->set_gain(0.625f);
                allpass2->set_gain(0.625f);
                allpass3->set_gain(0.625f);
                allpass4->set_gain(0.625f);
                del_a->set_modulation_rate(0.3f*(1.f/SAMPLE_RATE));
                del_a->set_sample_depth(100);
                allpass1_a->set_gain(0.625f);
                allpass2_a->set_gain(0.625f);
                del_b->set_modulation_rate(0.35f*(1.f/SAMPLE_RATE));
                del_b->set_sample_depth(100);
                allpass1_b->set_gain(0.625f);
                allpass2_b->set_gain(0.625f);
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            void process(const BufferInterface& input, PolyBufferInterface& output)
            {
                for(size_t i = 0; i < input.size(); i++)
                    _process(input[i], output[0][i], output[1][i]);
            }

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            void process_for(const BufferInterface& input, PolyBufferInterface& output)
            {
                for(size_t i = 0; i < N; i++)
                    _process(input[i], output[0][i], output[1][i]);
            }

            constexpr
            inline void set_gain(Sample g) {gain = g;}

            private:
            void _process(const Sample input, Sample& output_left, Sample& output_right)
            {
                //diffusion
                Sample ap_out1 = allpass1->process(input);
                Sample ap_out2 = allpass2->process(ap_out1);
                Sample ap_out3 = allpass3->process(ap_out2);
                Sample ap_out4 = allpass4->process(ap_out3);
                //branch_a
                Sample damp_a = dampening_a.process(ap_out4 + feedback_b);
                Sample mod_a = del_a->process(damp_a);
                Sample ap1a_out = allpass1_a->process(mod_a);
                Sample ap2a_out = allpass2_a->process(ap1a_out);
                feedback_a = ap2a_out * gain;
                //branch_b
                Sample damp_b = dampening_b.process(ap_out4 + feedback_a);
                Sample mod_b = del_b->process(damp_b);
                Sample ap1b_out = allpass1_b->process(mod_b);
                Sample ap2b_out = allpass2_b->process(ap1b_out);
                feedback_b = ap2b_out * gain;

                output_left = feedback_a;
                output_right = feedback_b;
            }


            ModulatedAllpass<SampleBufferStatic<static_cast<size_t>(4.76f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> allpass1;
            Allpass<SampleBufferStatic<static_cast<size_t>(3.58f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> allpass2;
            Allpass<SampleBufferStatic<static_cast<size_t>(12.721f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> allpass3;
            Allpass<SampleBufferStatic<static_cast<size_t>(9.297f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> allpass4;

            ModulatedDelay<SampleBufferStatic<static_cast<size_t>(106.59375f*(static_cast<float>(SAMPLE_RATE)/1000.f))>> del_a;
            OnepoleFilter dampening_a;
            Allpass<SampleBufferStatic<static_cast<size_t>(51.625f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> allpass1_a;
            Allpass<SampleBufferStatic<static_cast<size_t>(63.68725f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> allpass2_a;

            ModulatedDelay<SampleBufferStatic<static_cast<size_t>(149.4375f*(static_cast<float>(SAMPLE_RATE)/1000.f))>> del_b;
            OnepoleFilter dampening_b;
            Allpass<SampleBufferStatic<static_cast<size_t>(59.78125f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> allpass1_b;
            Allpass<SampleBufferStatic<static_cast<size_t>(51.96875f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> allpass2_b;

            Sample gain;
            Sample feedback_a;
            Sample feedback_b;
    };

    template<size_t SAMPLE_RATE>
    class FDN_4
    {
        using FilterType = typename idsp::OnepoleFilter::Type;

        public:
            FDN_4() :
            dampening1{FilterType::Lowpass, (6500.f*(1.f/SAMPLE_RATE))},
            dampening2{FilterType::Lowpass, (6500.f*(1.f/SAMPLE_RATE))},
            dampening3{FilterType::Lowpass, (6500.f*(1.f/SAMPLE_RATE))},
            dampening4{FilterType::Lowpass, (6500.f*(1.f/SAMPLE_RATE))},
            decay{0}
            {}

            IDSP_CONSTEXPR_SINCE_CXX14
            /**assumes left/right/in/out buffers are all of equal length*/
            inline void process(const PolyBufferInterface& input, PolyBufferInterface& output)
            {
                for(size_t i = 0; i < input[0].size(); i++)
                    process(input[0][i], input[1][i], output[0][i], output[1][i]);
            }

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            inline void process_for(const PolyBufferInterface& input, PolyBufferInterface& output)
            {
                for(size_t i = 0; i < N; i++)
                    process(input[0][i], input[1][i], output[0][i], output[1][i]);
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            void process(const Sample in_l, const Sample in_r, Sample& out_l, Sample& out_r)
            {
                // delay lines
                matrix_in[0] = delay1->read();
                matrix_in[1] = delay2->read();
                matrix_in[2] = delay3->read();
                matrix_in[3] = delay4->read();
                delay1->write(matrix_out[0]);
                delay2->write(matrix_out[1]);
                delay3->write(matrix_out[2]);
                delay4->write(matrix_out[3]);
                //dampening
                matrix_in[0] = dampening1.process(matrix_in[0]);
                matrix_in[1] = dampening2.process(matrix_in[1]);
                matrix_in[2] = dampening3.process(matrix_in[2]);
                matrix_in[3] = dampening4.process(matrix_in[3]);
                // input
                matrix_in[0] += in_l;
                matrix_in[1] += in_l;
                // matrix mixing
                float M1 = (matrix_in[0] * decay) + (matrix_in[1] * decay);
                float M2 = (matrix_in[0] * decay) - (matrix_in[1] * decay);
                float M3 = (matrix_in[2] * decay) + (matrix_in[3] * decay);
                float M4 = (matrix_in[2] * decay) - (matrix_in[3] * decay);
                matrix_out[0] = M1 + M3;
                matrix_out[1] = M2 + M4;
                matrix_out[2] = M1 - M3;
                matrix_out[3] = M2 - M4;
                // output
                out_l = matrix_in[0] * decay;
                out_r = matrix_in[1] * decay;
            }

            inline void set_dampening(float f)
            {
                dampening1.set_cutoff(f * (1.f/SAMPLE_RATE));
                dampening2.set_cutoff(f * (1.f/SAMPLE_RATE));
                dampening3.set_cutoff(f * (1.f/SAMPLE_RATE));
                dampening4.set_cutoff(f * (1.f/SAMPLE_RATE));
            }

            inline void set_decay(float f) {decay = f * 0.5f;}

        private:
            OnepoleFilter dampening1;
            OnepoleFilter dampening2;
            OnepoleFilter dampening3;
            OnepoleFilter dampening4;

            float decay;

            Delay<SampleBufferStatic<static_cast<size_t>(58.6435f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> delay1;
            Delay<SampleBufferStatic<static_cast<size_t>(69.4325f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> delay2;
            Delay<SampleBufferStatic<static_cast<size_t>(74.5234f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> delay3;
            Delay<SampleBufferStatic<static_cast<size_t>(86.1244f * (static_cast<float>(SAMPLE_RATE)/1000.f))>> delay4;

            std::array<Sample, 4> matrix_in;
            std::array<Sample, 4> matrix_out;

    };



}// namespace idsp

#endif