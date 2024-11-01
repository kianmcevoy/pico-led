#ifndef IDSP_REVERB_TOOLKIT_H
#define IDSP_REVERB_TOOLKIT_H

#include "idsp/buffer_interface.hpp"
#include "idsp/ringbuffer.hpp"
#include "idsp/wrapper.hpp"
#include "idsp/modulation.hpp"
#include "idsp/buffer_types.hpp"

#include <array>

namespace idsp
{
    /**
     * Allpass Filter
     * */
    class AllpassProcessor
    {
        public:
            constexpr
            AllpassProcessor(BufferInterface& buff) :
            delay{AudioRingBuffer(buff)},
            gain{0.5}
            {}

            IDSP_CONSTEXPR_SINCE_CXX20
            ~AllpassProcessor() = default;

            IDSP_CONSTEXPR_SINCE_CXX14
            Sample process(const Sample input)
            {
                return this->_process_sample(input);
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            void process(const BufferInterface& input, BufferInterface& output)
            {
                for (size_t i = 0; i < input.size(); i++)
                    output[i] = this->_process_sample(input[i]);
            }

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            void process_for(const BufferInterface& input, BufferInterface& output)
            {
                for (size_t i = 0; i < N; i++)
                    output[i] = this->_process_sample(input[i]);
            }

            constexpr
            Sample read_offset(size_t offset) {return delay.read_offset(offset);}

            constexpr
            void set_gain(float g) {gain = g;}

        private:

            IDSP_CONSTEXPR_SINCE_CXX14
            Sample _process_sample(const Sample input)
            {
                // read the delay line to get w(n-D)
                Sample wnD = delay.read();
                // form w(n) = x(n) + gw(n-D)
                Sample wn = input + gain * wnD;
                // form y(n) = -gw(n) + w(n-D)
                Sample yn = -gain*wn + wnD;
                // write delay line
                delay.write(wn);
                return yn;
            }

            AudioRingBuffer delay;
            float gain;
    };

    template<class BufferT>
    using Allpass = ProcessWrapper<AllpassProcessor, BufferT>;

    /**
     * Mudulated-Allpass Filter
     * */
    class ModulatedAllpassProcessor
    {
        public:
            ModulatedAllpassProcessor(BufferInterface& buff) :
            delay{AudioRingBuffer(buff)},
            gain{0.5},
            sample_depth{0},
            delay_samples{0},
            lfo(idsp::Waveform::triangle, false)
            {}

            IDSP_CONSTEXPR_SINCE_CXX20
            ~ModulatedAllpassProcessor() = default;

            Sample process(const Sample input)
            {
                return this->_process_sample(input);
            }

            void process(const BufferInterface& input, BufferInterface& output)
            {
                for (size_t i = 0; i < input.size(); i++)
                    output[i] = this->_process_sample(input[i]);
            }

            template<size_t N>
            void process_for(const BufferInterface& input, BufferInterface& output)
            {
                for (size_t i = 0; i < N; i++)
                    output[i] = this->_process_sample(input[i]);
            }

            constexpr
            Sample read_offset(size_t offset) {return delay.read_offset(offset);}

            constexpr
            void set_gain(float g) {gain = g;}

            constexpr
            void set_modulation_depth(float f) {sample_depth *= f;}

            constexpr
            void set_sample_depth(float f) {sample_depth = f;}

            constexpr
            void set_modulation_rate(float f) {lfo.set_rate(f);}

        private:

            Sample _process_sample(const Sample input)
            {
                //calculate modulation
                Sample modulation = lfo.process() * sample_depth;
                if(modulation > static_cast<float>(delay.get_size()-1)) modulation -= (delay.get_size()-1);
                // read the delay line to get w(n-D)
                Sample wnD = delay.read_offset_smooth_wrap(static_cast<float>(delay.get_size()) - modulation);
                // form w(n) = x(n) + gw(n-D)
                Sample wn = input + gain * wnD;
                // form y(n) = -gw(n) + w(n-D)
                Sample yn = -gain*wn + wnD;
                // write delay line
                delay.write(wn);
                return yn;
            }

            AudioRingBuffer delay;
            float gain;
            float sample_depth;
            float delay_samples;
            WavetableOscillator<128> lfo;
    };

    template<class BufferT>
    using ModulatedAllpass = ProcessWrapper<ModulatedAllpassProcessor, BufferT>;


    /**
     * Nested Allpass Filter
     * */
    class NestedAllpassProcessor
    {
        public:
            constexpr
            NestedAllpassProcessor(BufferInterface& buffer, BufferInterface& buffer1) :
            delay{AudioRingBuffer(buffer)},
            allpass{AllpassProcessor(buffer1)},
            gain{0.5}
            {}

            IDSP_CONSTEXPR_SINCE_CXX20
            ~NestedAllpassProcessor() = default;

            IDSP_CONSTEXPR_SINCE_CXX14
            Sample process(const Sample input)
            {
                return this->_process_sample(input);
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            void process(const BufferInterface& input, BufferInterface& output)
            {
                for (size_t i = 0; i < input.size(); i++)
                    output[i] = this->_process_sample(input[i]);
            }

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            void process_for(const BufferInterface& input, BufferInterface& output)
            {
                for (size_t i = 0; i < N; i++)
                    output[i] = this->_process_sample(input[i]);
            }

            constexpr
            Sample read_offset(size_t offset) {return delay.read_offset(offset);}

            constexpr
            void set_gain(float g0, float g1)
            {
                gain = g0;
                allpass.set_gain(g1);
            }

        private:

            IDSP_CONSTEXPR_SINCE_CXX14
            Sample _process_sample(const Sample input)
            {
                // read the delay line to get w(n-D)
                Sample wnD = delay.read();
                // form w(n) = x(n) + gw(n-D)
                Sample wn = input + gain * wnD;
                // process wn through inner APF
                Sample y_inner = allpass.process(wn);
                // form y(n) = -gw(n) + w(n-D)
                Sample yn = -gain*wn + wnD;
                // write delay line
                delay.write(y_inner);
                return yn;
            }

            AudioRingBuffer delay;
            AllpassProcessor allpass;
            float gain;
    };

    template<class BufferT, class NestedBufferT>
    class NestedAllpass
    {
        public:
            NestedAllpass():
            processor{NestedAllpassProcessor(buffer, nested_buffer)}
            {}

            ~NestedAllpass() = default;

            NestedAllpassProcessor* operator->()
                { return &this->processor; }
            const NestedAllpassProcessor* operator->() const
                { return &this->processor; }

        private:
            BufferT buffer;
            NestedBufferT nested_buffer;
            NestedAllpassProcessor processor;
    };


    /**
     * Double Nested Allpass Filter
     * */
    class DoubleNestedAllpassProcessor
    {
        public:
            constexpr
            DoubleNestedAllpassProcessor(BufferInterface& buffer, BufferInterface& buffer1, BufferInterface& buffer2) :
            delay{AudioRingBuffer(buffer)},
            allpass1{AllpassProcessor(buffer1)},
            allpass2{AllpassProcessor(buffer2)},
            gain{0.5}
            {}

            IDSP_CONSTEXPR_SINCE_CXX20
            ~DoubleNestedAllpassProcessor() = default;

            IDSP_CONSTEXPR_SINCE_CXX14
            Sample process(const Sample input)
            {
                return this->_process_sample(input);
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            void process(const BufferInterface& input, BufferInterface& output)
            {
                for (size_t i = 0; i < input.size(); i++)
                    output[i] = this->_process_sample(input[i]);
            }

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
            void process_for(const BufferInterface& input, BufferInterface& output)
            {
                for (size_t i = 0; i < N; i++)
                    output[i] = this->_process_sample(input[i]);
            }

            constexpr
            Sample read_offset(size_t offset) {return delay.read_offset(offset);}

            constexpr
            void set_gain(float g0, float g1, float g2)
            {
                gain = g0;
                allpass1.set_gain(g1);
                allpass2.set_gain(g2);
            }

        private:

            IDSP_CONSTEXPR_SINCE_CXX14
            Sample _process_sample(const Sample input)
            {
                // read the delay line to get w(n-D)
                Sample wnD = delay.read();
                // form w(n) = x(n) + gw(n-D)
                Sample wn = input + gain * wnD;
                // process wn through inner APF
                Sample y_inner = allpass1.process(wn);
                // process y_inner through 2nd APF
                Sample y_outer = allpass2.process(y_inner);
                // form y(n) = -gw(n) + w(n-D)
                Sample yn = -gain*wn + wnD;
                // write delay line
                delay.write(y_outer);
                return yn;
            }

            AudioRingBuffer delay;
            AllpassProcessor allpass1;
            AllpassProcessor allpass2;
            float gain;
    };

    template<class BufferT, class Buffer1T, class Buffer2T>
    class DoubleNestedAllpass
    {
        public:
            DoubleNestedAllpass():
            processor{DoubleNestedAllpassProcessor(buffer, buffer1, buffer2)}
            {}

            ~DoubleNestedAllpass() = default;

            DoubleNestedAllpassProcessor* operator->()
                { return &this->processor; }
            const DoubleNestedAllpassProcessor* operator->() const
                { return &this->processor; }

        private:
            BufferT buffer;
            Buffer1T buffer1;
            Buffer2T buffer2;
            DoubleNestedAllpassProcessor processor;
    };

    /**
    * Delay
    */
    template<class BufferT>
    using Delay = ProcessWrapper<AudioRingBuffer, BufferT>;

    /**
    * Modulated Delay
    */
    class ModDelayProcessor
    {
        public:
            ModDelayProcessor(BufferInterface& buff) :
            delay{AudioRingBuffer(buff)},
            lfo(idsp::Waveform::triangle, false)
            {}

            // IDSP_CONSTEXPR_SINCE_CXX20
            ~ModDelayProcessor() = default;

            // IDSP_CONSTEXPR_SINCE_CXX14
            Sample process(const Sample input)
            {
                return this->_process_sample(input);
            }

            // IDSP_CONSTEXPR_SINCE_CXX14
            void process(const BufferInterface& input, BufferInterface& output)
            {
                for (size_t i = 0; i < input.size(); i++)
                    output[i] = this->_process_sample(input[i]);
            }

            template<size_t N>
            // IDSP_CONSTEXPR_SINCE_CXX14
            void process_for(const BufferInterface& input, BufferInterface& output)
            {
                for (size_t i = 0; i < N; i++)
                    output[i] = this->_process_sample(input[i]);
            }

            constexpr
            inline Sample read_offset(size_t offset) {return delay.read_offset(offset);}

            constexpr
            inline void set_modulation_depth(float f) {modulation_depth = clamp(f, 0.f, 1.f);}

            constexpr
            inline void set_sample_depth(float f) {sample_depth = clamp(f, 1.f, static_cast<float>(delay.get_size()) -1.f);}

            constexpr
            inline void set_modulation_rate(float f) {lfo.set_rate(f);}

            constexpr
            inline size_t get_size() {return delay.get_size();}

        private:
            inline Sample _process_sample(const Sample input)
            {
                Sample modulation = lfo.process() * (sample_depth * modulation_depth);
                if(modulation > static_cast<float>(delay.get_size()-1)) modulation -= (delay.get_size()-1);
                Sample output = delay.read_offset_smooth_wrap(static_cast<float>(delay.get_size()) - modulation);
                delay.write(input);
                return output;
            }

            AudioRingBuffer delay;
            float modulation_depth;
            float sample_depth;
            float delay_samples;
            WavetableOscillator<128> lfo;

    };
    template<class BufferT>
    using ModulatedDelay = ProcessWrapper<ModDelayProcessor, BufferT>;

    template<size_t SAMPLE_RATE>
	class DiffuserRev3
	{
		public:
			DiffuserRev3()
			{
				delay[0]->set_length(static_cast<size_t>(1.42763f * (SAMPLE_RATE)/1000.f));
				delay[1]->set_length(static_cast<size_t>(3.23873f * (SAMPLE_RATE)/1000.f));
				delay[2]->set_length(static_cast<size_t>(5.2345f * (SAMPLE_RATE)/1000.f));
				delay[3]->set_length(static_cast<size_t>(7.82312f * (SAMPLE_RATE)/1000.f));
			}

            template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
			inline void process_for(const PolyBufferInterface &input, PolyBufferInterface &output)
			{
				for(size_t i = 0; i < N; i++)
					process(input[0][i], input[1][i], output[0][i], output[1][i]);
			}

			inline void process(const Sample in_l, const Sample in_r, Sample& out_l, Sample& out_r)
			{
				for(size_t tap = 0; tap < num_taps; tap++)
					out[tap] = delay[tap]->read();

				out_l = ((out[2] + (out[1] + (out[0] + in_l))) * normalisation);
				out_r = out[3] * normalisation;

				delay[0]->write(in_r);
				delay[1]->write(in_l - out[0]);
				delay[2]->write((out[0] + in_l) - out[1]);
				delay[3]->write((out[1] + (out[0] + in_l)) - out[2]);
			}

		private:
			static constexpr size_t num_taps = 4;
			std::array<Delay<SampleBufferStatic<static_cast<size_t>(7.82312f * (static_cast<float>(SAMPLE_RATE)/1000.f))>>, num_taps> delay;
			std::array<Sample, num_taps> out;
			static constexpr float normalisation = 0.34;
	};


    template<size_t SAMPLE_RATE>
    class DiffuserRev2
    {
        public:
            DiffuserRev2()
            {
                delay[0]->set_length(static_cast<size_t>(43.5337f * (SAMPLE_RATE)/1000.f));
                delay[1]->set_length(static_cast<size_t>(25.796f * (SAMPLE_RATE)/1000.f));
                delay[2]->set_length(static_cast<size_t>(19.392f * (SAMPLE_RATE)/1000.f));
                delay[3]->set_length(static_cast<size_t>(16.364f * (SAMPLE_RATE)/1000.f));
                delay[4]->set_length(static_cast<size_t>(7.645f * (SAMPLE_RATE)/1000.f));
                delay[5]->set_length(static_cast<size_t>(4.2546f * (SAMPLE_RATE)/1000.f));
            }

			template<size_t N>
            IDSP_CONSTEXPR_SINCE_CXX14
			inline void process_for(const PolyBufferInterface &input, PolyBufferInterface &output)
			{
				for(size_t i = 0; i < N; i++)
					process(input[0][i], input[1][i], output[0][i], output[1][i]);
			}

			inline void process(const Sample in_l, Sample in_r, Sample& out_l, Sample& out_r)
			{
				for(size_t tap = 0; tap < num_taps; tap++)
					out[tap] = delay[tap]->read();

				out_l = (out[4] + (out[3] + (out[2] +(out[1] +(out[0] + in_l))))) * normalisation;
				out_r = out[5] * normalisation;

				delay[0]->write(in_r);
				delay[1]->write(in_l - out[0]);
				delay[2]->write((out[0] + in_l) - out[1]);
				delay[3]->write((out[1] + (out[0] + in_l)) - out[2]);
				delay[4]->write((out[2] + (out[1] + (out[0] + in_l))) - out[3]);
				delay[5]->write((out[3] + (out[2] + (out[1] + (out[0] + in_l)))) - out[4]);
			}

        private:
            static constexpr size_t num_taps = 6;
            std::array<Delay<SampleBufferStatic<static_cast<size_t>(43.5337f  * (static_cast<float>(SAMPLE_RATE)/1000.f))>>, num_taps> delay;
            std::array<Sample, num_taps> out;
            static constexpr float normalisation = 0.28;
    };

} // namespace idsp

#endif