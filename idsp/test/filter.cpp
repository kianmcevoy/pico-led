#ifdef SYSTEM_RPI3
    #undef SYSTEM_RPI3
#endif
#ifdef SYSTEM_MP1
    #undef SYSTEM_MP1
#endif

#include "idsp/filter.hpp"
#include "testers.hpp"

#include "idsp/buffer_types.hpp"

#include <random>

static constexpr size_t dsp_block_size = 64;
static constexpr float sample_rate = 48000;

using Buffer = idsp::SampleBufferStatic<dsp_block_size>;

static void fill_buffer(idsp::BufferInterface& buffer)
{
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_real_distribution<Sample> dist(-1, 1);
    for (auto& x : buffer)
        x = dist(eng);
}

int main(int argc, const char* argv[])
{
    Buffer buffer;

    {
        fill_buffer(buffer);
        Buffer buffer1 = buffer.copy();
        // Shorthand
        using FilterType = idsp::OnepoleFilter::Type;
        idsp::OnepoleFilter lop(FilterType::Lowpass, 440 / sample_rate);
        lop.process(buffer.interface(), buffer1.interface());

        buffer = buffer1.copy();
        lop.set_cutoff(220 / sample_rate);
        for (auto& x : buffer)
            x = lop.process(x);
    }

    {
        Buffer buffer1;
        idsp::BufferInterface buffer1_ref = buffer1.interface();
        buffer1_ref = buffer.copy();
        buffer.copy_for<dsp_block_size>(buffer1_ref.copy());

        fill_buffer(buffer);
        // Shorthand
        using FilterType = idsp::BiquadFilter::Type;
        idsp::BiquadFilter bpf(FilterType::Bandpass, 250 / sample_rate, 0.5);
        bpf.process(buffer.interface(), buffer1_ref);

        buffer = buffer1_ref.copy();
        idsp::BiquadFilter notch(FilterType::Notch, 250 / sample_rate, 0.9);
        for (auto& x : buffer)
            x = notch.process(x);
    }

    return 0;
}
