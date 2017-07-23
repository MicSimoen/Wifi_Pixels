#ifndef WIFI_PIXELS_LED_H
#define WIFI_PIXELS_LED_H

#include "parameters.h"
#include "config.h"

#include <FastLED.h>

class wifi_pixels_led {
private:
    led_effect current_effect;
    CRGB leds[NUM_LEDS];
    uint8_t rainbow_step;
    uint8_t comet_hue;
public:
    wifi_pixels_led(void);

    void set_current_effect(led_effect new_current_effect);
    led_effect get_current_effect(void);
    void set_comet_hue(uint8_t new_comet_hue);
    uint8_t get_comet_hue(void);
    void increment_comet_hue(void);

    void play_current_effect(unsigned long 	frame_delay);

    void leds_off(void);
    void rainbow_effect(void);
    void comet_effect(void);
};

#endif
