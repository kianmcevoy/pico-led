#ifndef IDSP_OSCILLATOR_H
#define IDSP_OSCILLATOR_H

#include "idsp/lookup.hpp"

namespace idsp
{
    enum class Waveform
    {
        sine,
        triangle,
        square,
        sawtooth,
        ramp,
    };

    class RawOscillator
    {
        using Generator = Sample (*)(Sample);

        public:
            constexpr
            RawOscillator(Generator generator):
            _rate{1},
            _phase{0},
            _offset{0},
            _generator{generator}
            {}

            constexpr
            RawOscillator(Waveform waveform, bool bipolar):
            RawOscillator([waveform, bipolar]() -> Generator {
                if (bipolar)
                {
                    switch (waveform)
                    {
                        case Waveform::sine:
                            return [](Sample p) -> Sample {
                                return std::sin(p * twopi);
                            };
                        case Waveform::triangle:
                            return [](Sample p) -> Sample {
                                return rescale(std::abs(p - 0.5f), 0.f, 0.5f, -1.f, 1.f);
                            };
                        case Waveform::square:
                            return [](Sample p) -> Sample {
                                return p < 0.5f ? 1.f : -1.f;
                            };
                        case Waveform::sawtooth:
                            return [](Sample p) -> Sample {
                                return rescale(p, 0.f, 1.f, 1.f, -1.f);
                            };
                        default:
                        case Waveform::ramp:
                            return [](Sample p) -> Sample {
                                return rescale(p, 0.f, 1.f, -1.f, 1.f);
                            };
                    }
                }
                else
                {
                    switch (waveform)
                    {
                        case Waveform::sine:
                            return [](Sample p) -> Sample {
                                return rescale(std::sin(p * twopi), -1.f, 1.f, 0.f, 1.f);
                            };
                        case Waveform::triangle:
                            return [](Sample p) -> Sample {
                                return std::abs(p - 0.5f) * 2.f;
                            };
                        case Waveform::square:
                            return [](Sample p) -> Sample {
                                return p < 0.5f ? 1.f : 0.f;
                            };
                        case Waveform::sawtooth:
                            return [](Sample p) -> Sample {
                                return -p;
                            };
                        default:
                        case Waveform::ramp:
                            return [](Sample p) -> Sample {
                                return p;
                            };
                    }
                }
            }())
            {}

            inline
            float process()
            {
                this->_phase += this->_rate;
                this->_phase = wrap(this->_phase);
                return this->_generator(this->_phase);
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            void set_rate(float rate)
            {
                this->_rate = rate;
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            void set_phase(float phase)
            {
                this->_phase = idsp::wrap(phase + this->_offset);
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            void set_phase_offset(float offset)
            {
                const float raw_phase = idsp::wrap(this->_phase - this->_offset);
                this->_phase = idsp::wrap(raw_phase + offset);
                this->_offset = offset;
            }

        private:
            float _rate;
            float _phase;
            float _offset;
            Generator _generator;
    };

    template<size_t Size>
    class WavetableOscillator
    {
        using Generator = Sample (*)(Sample);
        using Wavetable = LookupTable<Sample, Size>;

        public:
            WavetableOscillator(const Wavetable& table):
            _rate{0},
            _phase{0},
            _offset{0},
            _table{table}
            {}

            WavetableOscillator(Wavetable&& table):
            _rate{0},
            _phase{0},
            _offset{0},
            _table{table}
            {}

            WavetableOscillator(Generator generator):
            WavetableOscillator(Wavetable(generator)) {}

            WavetableOscillator(Waveform waveform, bool bipolar):
            WavetableOscillator([waveform, bipolar]() -> Generator {
                if (bipolar)
                {
                    switch (waveform)
                    {
                        default:
                        case Waveform::sine:
                            return [](Sample p) -> Sample {
                                return std::sin(p * twopi);
                            };
                        case Waveform::triangle:
                            return [](Sample p) -> Sample {
                                if (p < 0.75f) return rescale(1.f - 2.f * std::abs(p - 0.25f), 0.f, 1.f, -1.f, 1.f);
                                else return rescale((2.f * (p - 0.75f)), 0.f, 1.f, -1.f, 1.f);
                            };
                        case Waveform::square:
                            return [](Sample p) -> Sample {
                                return p < 0.5f ? 1.f : -1.f;
                            };
                        case Waveform::sawtooth:
                            return [](Sample p) -> Sample {
                                return rescale(p, 0.f, 1.f, 1.f, -1.f);
                            };
                        case Waveform::ramp:
                            return [](Sample p) -> Sample {
                                return rescale(p, 0.f, 1.f, -1.f, 1.f);
                            };
                    }
                }
                else
                {
                    switch (waveform)
                    {
                        default:
                        case Waveform::sine:
                            return [](Sample p) -> Sample {
                                return rescale(std::sin(p * twopi), -1.f, 1.f, 0.f, 1.f);
                            };
                        case Waveform::triangle:
                            return [](Sample p) -> Sample {
                                if (p < 0.75f) return 1.f - 2.f * std::abs(p - 0.25f);
                                else return 2.f * (p - 0.75f);
                            };
                        case Waveform::square:
                            return [](Sample p) -> Sample {
                                return p < 0.5f ? 1.f : 0.f;
                            };
                        case Waveform::sawtooth:
                            return [](Sample p) -> Sample {
                                return 1.f - p;
                            };
                        case Waveform::ramp:
                            return [](Sample p) -> Sample {
                                return p;
                            };
                    }
                }
            }()) {}

            template<size_t N>
            void process_for(BufferInterface& output)
            {
                for(size_t i = 0; i < N; i++)
                    output[i] = this->process();
            }

            inline float process()
            {
                this->_phase += this->_rate;
                this->_phase = wrap(this->_phase);
                return this->_table.read(this->_phase);
            }

            inline float process_oneshot()
            {
                this->_phase += this->_rate;
                if (this->_phase >= 1.f) return this->_table.read(1.f);
                else return this->_table.read(this->_phase);
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            void set_rate(float rate)
            {
                this->_rate = rate;
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            void set_phase(float phase)
            {
                this->_phase = idsp::wrap(phase + this->_offset);
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            float get_phase()
            {
                return this->_phase;
            }

            IDSP_CONSTEXPR_SINCE_CXX14
            void set_phase_offset(float offset)
            {
                const float raw_phase = idsp::wrap(this->_phase - this->_offset);
                this->_phase = idsp::wrap(raw_phase + offset);
                this->_offset = offset;
            }


        private:
            float _rate;
            float _phase;
            float _offset;
            Wavetable _table;
    };
} // namespace idsp

#endif
