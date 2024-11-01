#ifndef IDSP_WRAPPER_H
#define IDSP_WRAPPER_H

namespace idsp
{
    template<class ProcessorT, class BufferT>
    class ProcessWrapper
    {
        public:
            ProcessWrapper():
            processor{ProcessorT(buffer)}
            {}

            ~ProcessWrapper() = default;

            ProcessorT* operator->()
                { return &this->processor; }
            const ProcessorT* operator->() const
                { return &this->processor; }

        private:
            BufferT buffer;
            ProcessorT processor;
    };

/**
 *
 * Example implementation of an effect, in this case named "EffectExample"
 * using the ProcessWrapper class to connect a BufferInterface to a Processor class.
 *
    template<class SampleT>
    class EffectExampleProcessor
    {
        public:
            EffectExampleProcessor(idsp::BufferInterface<SampleT>& buff) :
            buffer{AudioRingBuffer<SampleT>(buff)}{}

            ~EffectExampleProcessor() = default;

            SampleT process(const SampleT input){return input;}

        private:
            idsp::AudioRingBuffer<SampleT> buffer;
    };

    template<class SampleT, class BufferT>
    using EffectExample = ProcessWrapper<EffectExampleProcessor<SampleT>, BufferT>;
  */

} // namespace idsp

#endif
