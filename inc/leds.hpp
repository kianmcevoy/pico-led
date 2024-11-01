#ifndef __LEDS_H
#define __LEDS_H

#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "idsp/functions.hpp"


enum class Colour
{
    ROSE,
    BLUE,
    CYAN,
    PURPLE,
    MAGENTA,
    YELLOW,
    RED,
    GREEN,
    ORANGE,
};

enum class Mode
{
    Ambient,
    Glitch,
    Synth,
    Strings,
};

static constexpr Mode operator++(Mode& m, int)
{
    const Mode result = m;
    m = (m == Mode::Strings) ? Mode::Ambient : static_cast<Mode>(static_cast<int>(m) + 1);
    return result;
}

static constexpr Mode& operator++(Mode& m)
{
    m = (m == Mode::Strings) ? Mode::Ambient : static_cast<Mode>(static_cast<int>(m) + 1);
    return m;
}

struct Pixel
{
    Pixel(int layer_id) :
    rgb{0, 0, 0},
    brightness{0.0},
    integer_brightness{0},
    ratchet_count{0},
    phase{0},
    duration{0},
    active{false},
    layer_id{layer_id}
    {}

    uint8_t rgb[3];
    float brightness;
    uint8_t integer_brightness;
    uint8_t ratchet_count;
    uint32_t phase;
    uint32_t duration;
    bool active;
    int layer_id;
};

enum class Menu
{
    Volume,
    VoiceCount,
    PitchShift,
    Sensitivity,
    MidiMode,
};

class Leds
{
    public:
        Leds() :
        pixel{{4},{3},{2},{1},{0},{1},{2},{3},{4}}
        {}

        void init();

        void startup_animation();

        void process();

        void update_menu(Menu menu, int value, float offset);

    private:
        void _volume_menu(int volume);
        
        void _sensitivity_menu(int sensitivity, float offset);
      
        void _voice_count_menu(int voice_count);
        
        void _pitch_shift_menu(int pitch_shift);
       
        void _midi_mode_menu(int midi_mode);
   
        void _set_colour(Colour colour);

        void _clear_leds();

        static inline void _put_pixel(uint32_t pixel_grb) {
            pio_sm_put_blocking(pio1, 0, pixel_grb << 8u);
        }

        static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b){
            return ((uint32_t) (r) << 8) | ((uint32_t) (g) << 16) | (uint32_t) (b);
        }

        static constexpr bool IS_RGBW = false;
        static constexpr int NUM_PIXELS = 9;
        static constexpr int NUM_LAYERS = 4;
        static constexpr int LEDS_PER_LAYER = 2;
        static constexpr int FRAME_RATE = 20;
        static constexpr uint8_t WS2812_PIN = 1;

        Pixel pixel[NUM_PIXELS];
};

#endif