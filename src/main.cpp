

#include "pico/stdlib.h"
#include "leds.hpp"

#define BUTTON_VOL_UP_GPIO 17
#define BUTTON_VOL_DOWN_GPIO 18
#define BUTTON_SENSITIVITY_UP_GPIO 21
#define BUTTON_SENSITIVITY_DOWN_GPIO 20
#define BUTTON_MODE_GPIO 19

static Leds leds;

static bool volume_up_previous_state = false;
static bool volume_down_previous_state = false;
static bool sensitivity_up_previous_state = false;
static bool sensitivity_down_previous_state = false;
static bool mode_previous_state = false;

static uint8_t volume = 0;
static uint8_t voice_count = 0;
static int pitch_shift = 0;

int main()
{
    leds.init();
    leds.startup_animation();

    gpio_init(BUTTON_VOL_UP_GPIO);
    gpio_set_dir(BUTTON_VOL_UP_GPIO, GPIO_IN);

    gpio_init(BUTTON_VOL_DOWN_GPIO);
    gpio_set_dir(BUTTON_VOL_DOWN_GPIO, GPIO_IN);

    gpio_init(BUTTON_SENSITIVITY_UP_GPIO);
    gpio_set_dir(BUTTON_SENSITIVITY_UP_GPIO, GPIO_IN);

    gpio_init(BUTTON_SENSITIVITY_DOWN_GPIO);
    gpio_set_dir(BUTTON_SENSITIVITY_DOWN_GPIO, GPIO_IN);

    gpio_init(BUTTON_MODE_GPIO);
    gpio_set_dir(BUTTON_MODE_GPIO, GPIO_IN);

    while(1) 
    {
        bool volume_up_state = gpio_get(BUTTON_VOL_UP_GPIO);
        bool volume_down_state = gpio_get(BUTTON_VOL_DOWN_GPIO);

        bool sensitivity_up_state = gpio_get(BUTTON_SENSITIVITY_UP_GPIO);
        bool sensitivity_down_state = gpio_get(BUTTON_SENSITIVITY_DOWN_GPIO);

        bool mode = !gpio_get(BUTTON_MODE_GPIO);

        if(volume_up_state && !volume_up_previous_state && !mode)
        {
            volume++;
            if(volume > 10) volume = 10;
            leds.update_menu(Menu::Volume, volume, 0);
        }
        else if(volume_down_state && !volume_down_previous_state && !mode)
        {
            if(volume > 0) volume--;
            leds.update_menu(Menu::Volume, volume, 0);
        }

        if(sensitivity_up_state && !sensitivity_up_previous_state && mode)
        {
            voice_count++;
            if(voice_count > 5) voice_count = 5;
            leds.update_menu(Menu::VoiceCount, voice_count, 0);
        }
        else if(sensitivity_down_state && !sensitivity_down_previous_state && mode)
        {
            if(voice_count > 0) voice_count--;
            leds.update_menu(Menu::VoiceCount, voice_count, 0);
        }

        if(volume_up_state && !volume_up_previous_state && mode)
        {
            pitch_shift++;
            if(pitch_shift > 12) pitch_shift = 12;
            leds.update_menu(Menu::PitchShift, pitch_shift, 0);
        }
        else if(volume_down_state && !volume_down_previous_state && mode)
        {
            if(pitch_shift > -12) pitch_shift--;
            leds.update_menu(Menu::PitchShift, pitch_shift, 0);
        }

        leds.process();

        volume_up_previous_state = volume_up_state;
        volume_down_previous_state = volume_down_state;
        sensitivity_up_previous_state = sensitivity_up_state;
        sensitivity_down_previous_state = sensitivity_down_state;
        mode_previous_state = mode;
    }
    return 0;
}