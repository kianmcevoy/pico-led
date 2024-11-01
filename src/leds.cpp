#include "leds.hpp"
#include <stdlib.h>
#include "colours.hpp"

static volatile bool frame_clock{false};
static volatile bool menu{false};
static volatile alarm_id_t menu_alarm_id{0};

int64_t frame_clock_callback(alarm_id_t id, void* user_data) {
    frame_clock = true;
    return 0;
}

int64_t menu_callback(alarm_id_t id, void* user_data) {
    menu = false;
    return 0;
}

void Leds::init()
{
    PIO pio = pio1;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
    _clear_leds();
}

void Leds::_clear_leds()
{
    for(int i = 0; i < NUM_PIXELS; i++)
    { 
        _put_pixel(urgb_u32(0,0,0));
    }
}

void Leds::startup_animation()
{
    float brightness;
    for(int scene = 0; scene < 4; scene++)
    {
        _set_colour(startup[scene]);

        for(int frame = 0; frame < 64; frame++)
        {
            if(frame < 32)
            {
                brightness = (float)frame/32.f;
            }
            else
            {
                brightness = (float)(64-frame)/32.f;
            }

            for(int pixel_id = 0; pixel_id < NUM_PIXELS; pixel_id++)
            {
                float brightness_weighting = 1.f/(6 - pixel[pixel_id].layer_id);
                float a = static_cast<float>(pixel[pixel_id].rgb[0]) * (brightness - brightness_weighting);
                float b = static_cast<float>(pixel[pixel_id].rgb[1]) * (brightness - brightness_weighting);
                float c = static_cast<float>(pixel[pixel_id].rgb[2]) * (brightness - brightness_weighting);

                _put_pixel(urgb_u32((uint8_t)a, (uint8_t)b, (uint8_t)c));
            }
            
            sleep_ms(20);      
        }
        add_alarm_in_ms(FRAME_RATE, frame_clock_callback, NULL, true);
    }
    _clear_leds();
}

void Leds::update_menu(Menu menu, int value, float offset)
{
    switch(menu)
    {
        case Menu::Volume:
            _volume_menu(value);
        break;

        case Menu::VoiceCount:
            _voice_count_menu(value);
        break;

        case Menu::PitchShift:
            _pitch_shift_menu(value);
        break;

        case Menu::Sensitivity:
            _sensitivity_menu(value, offset);
        break;

        case Menu::MidiMode:
            _midi_mode_menu(value);
        break;
    }
}

void Leds::_volume_menu(int volume)
{
    menu = true;
    uint8_t rgb[3]{85, 85, 0};
    for(int i = 0; i < NUM_PIXELS; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            pixel[i].rgb[j] = idsp::max((rgb[j] - brightness[volume][pixel[i].layer_id]),0);
        }
        _put_pixel(urgb_u32(pixel[i].rgb[0], pixel[i].rgb[1], pixel[i].rgb[2]));
    }
    if(menu_alarm_id != 0) cancel_alarm(menu_alarm_id);
    menu_alarm_id = add_alarm_in_ms(2000, menu_callback, NULL, true);
}

void Leds::_voice_count_menu(int voice_count)
{
    menu = true;
    uint8_t rgb[3]{0, 0, 85};
    for(int i = 0; i < NUM_PIXELS; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            pixel[i].rgb[j] = idsp::max((rgb[j] - brightness[voice_count*2][pixel[i].layer_id]),0);
        }
        _put_pixel(urgb_u32(pixel[i].rgb[0], pixel[i].rgb[1], pixel[i].rgb[2]));
    }
    if(menu_alarm_id != 0) cancel_alarm(menu_alarm_id);
    menu_alarm_id = add_alarm_in_ms(2000, menu_callback, NULL, true);
}

void Leds::_pitch_shift_menu(int pitch_shift)
{
    uint8_t level = ((pitch_shift+12)/2);
    level = idsp::clamp<uint8_t>(level, 1, 10);
    menu = true;
    uint8_t rgb[3]{85, 0, 0};
    for(int i = 0; i < NUM_PIXELS; i++)
    {
        for(int j = 0; j < 3; j++){
            pixel[i].rgb[j] = idsp::max((rgb[j] - brightness[level][pixel[i].layer_id]),0);
        }

        if(pitch_shift == -12){
            pixel[4].rgb[0] = 85;
            pixel[4].rgb[1] = 85;
            pixel[4].rgb[2] = 0;
        }
        if(pitch_shift == -11){
            pixel[4].rgb[0] = 60;
            pixel[4].rgb[1] = 60;
            pixel[4].rgb[2] = 0;
        }
        if(pitch_shift == 11){
            pixel[0].rgb[0] = 60;
            pixel[0].rgb[1] = 60;
            pixel[0].rgb[2] = 0;
            pixel[8].rgb[0] = 60;
            pixel[8].rgb[1] = 60;
            pixel[8].rgb[2] = 0;
        }
        if(pitch_shift == 12){
            pixel[0].rgb[0] = 85;
            pixel[0].rgb[1] = 85;
            pixel[0].rgb[2] = 85;
            pixel[8].rgb[0] = 85;
            pixel[8].rgb[1] = 85;
            pixel[8].rgb[2] = 85;
        }
        _put_pixel(urgb_u32(pixel[i].rgb[0], pixel[i].rgb[1], pixel[i].rgb[2]));
    }
    if(menu_alarm_id != 0) cancel_alarm(menu_alarm_id);
    menu_alarm_id = add_alarm_in_ms(2000, menu_callback, NULL, true);
}

void Leds::_sensitivity_menu(int sensitivity, float offset)
{
    menu = true;
    uint8_t rgb[3]{0, 85, 0};
    for(int i = 0; i < NUM_PIXELS; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            pixel[i].rgb[j] = idsp::max<int>((rgb[j] - idsp::interpolate_2<float>(offset, brightness[sensitivity][pixel[i].layer_id], brightness[sensitivity+1][pixel[i].layer_id])),0);
        }
        _put_pixel(urgb_u32(pixel[i].rgb[0], pixel[i].rgb[1], pixel[i].rgb[2]));
    }
    if(menu_alarm_id != 0) cancel_alarm(menu_alarm_id);
    menu_alarm_id = add_alarm_in_ms(2000, menu_callback, NULL, true);
}

void Leds::_midi_mode_menu(int mode)
{
    menu = true;
    uint8_t rgb[3];
    rgb[0] = mode ? 255 : 0;
    rgb[1] = mode ? 0 : 255;
    rgb[2] = mode ? 0 : 0;
    for(int i = 0; i < NUM_PIXELS; i++)
    {
        for(int j = 0; j < 3; j++)
        {
            pixel[i].rgb[j] = idsp::max((rgb[j] - brightness[5*2][pixel[i].layer_id]),0);
        }
        _put_pixel(urgb_u32(pixel[i].rgb[0], pixel[i].rgb[1], pixel[i].rgb[2]));
    }
    if(menu_alarm_id != 0) cancel_alarm(menu_alarm_id);
    menu_alarm_id = add_alarm_in_ms(2000, menu_callback, NULL, true);
}

void Leds::_set_colour(Colour colour)
{
    switch(colour)
    {
        case Colour::ROSE:
            for (int pixel_id = 0; pixel_id < NUM_PIXELS; pixel_id++)
            {
                for(int i = 0; i < 3; i++)
                {
                    uint8_t layer = NUM_LAYERS - pixel[pixel_id].layer_id;
                    pixel[pixel_id].rgb[i] = rose[layer][i];
                }
            }
        break;

        case Colour::BLUE:
            for (int pixel_id = 0; pixel_id < NUM_PIXELS; pixel_id++)
            {
                for(int i = 0; i < 3; i++)
                {
                    uint8_t layer = NUM_LAYERS - pixel[pixel_id].layer_id;
                    pixel[pixel_id].rgb[i] = blue[layer][i];
                }
            }
        break;

        case Colour::PURPLE:
            for (int pixel_id = 0; pixel_id < NUM_PIXELS; pixel_id++)
            {
                for(int i = 0; i < 3; i++)
                {
                    uint8_t layer = NUM_LAYERS - pixel[pixel_id].layer_id;
                    pixel[pixel_id].rgb[i] = purple[layer][i];
                }
            }
        break;

        case Colour::CYAN:
            for (int pixel_id = 0; pixel_id < NUM_PIXELS; pixel_id++)
            {
                for(int i = 0; i < 3; i++)
                {
                    uint8_t layer = NUM_LAYERS - pixel[pixel_id].layer_id;
                    pixel[pixel_id].rgb[i] = cyan[layer][i];
                }
            }
        break;

        case Colour::MAGENTA:
            for (int pixel_id = 0; pixel_id < NUM_PIXELS; pixel_id++)
            {
                for(int i = 0; i < 3; i++)
                {
                    uint8_t layer = NUM_LAYERS - pixel[pixel_id].layer_id;
                    pixel[pixel_id].rgb[i] = magenta[layer][i];
                }
            }
        break;

        case Colour::YELLOW:
            for (int pixel_id = 0; pixel_id < NUM_PIXELS; pixel_id++)
            {
                for(int i = 0; i < 3; i++)
                {
                    uint8_t layer = NUM_LAYERS - pixel[pixel_id].layer_id;
                    pixel[pixel_id].rgb[i] = yellow[layer][i];
                }
            }
        break;

        case Colour::RED:
            for (int pixel_id = 0; pixel_id < NUM_PIXELS; pixel_id++)
            {
                for(int i = 0; i < 3; i++)
                {
                    uint8_t layer = NUM_LAYERS - pixel[pixel_id].layer_id;
                    pixel[pixel_id].rgb[i] = red[layer][i];
                }
            }
        break;

        case Colour::GREEN:
            for (int pixel_id = 0; pixel_id < NUM_PIXELS; pixel_id++)
            {
                for(int i = 0; i < 3; i++)
                {
                    uint8_t layer = NUM_LAYERS - pixel[pixel_id].layer_id;
                    pixel[pixel_id].rgb[i] = green[layer][i];
                }
            }
        break;

        case Colour::ORANGE:
            for (int pixel_id = 0; pixel_id < NUM_PIXELS; pixel_id++)
            {
                for(int i = 0; i < 3; i++)
                {
                    uint8_t layer = NUM_LAYERS - pixel[pixel_id].layer_id;
                    pixel[pixel_id].rgb[i] = orange[layer][i];
                }
            }
        break;
    }
}

void Leds::process()
{

}