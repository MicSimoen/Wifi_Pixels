#ifndef WIFI_PIXELS_LED_H
#define WIFI_PIXELS_LED_H

#include "parameters.h"
#include "config.h"
#define FASTLED_ALLOW_INTERRUPTS 0 //To resolve the led flickering when driving with ESP8266
#include <FastLED.h>

class wifi_pixels_led {
private:
led_effect current_effect;
CRGB leds[NUM_LEDS];
uint8_t rainbow_step;
uint8_t hue;
uint8_t brightness;
bool police_phase;
public:
wifi_pixels_led(void);

void set_current_effect(led_effect new_current_effect);
led_effect get_current_effect(void);
void set_hue(uint8_t new_hue);
uint8_t get_hue(void);
void increment_hue(void);
void set_brightness(uint8_t new_brightness);
uint8_t get_brightness(void);

void play_current_effect(unsigned long frame_delay);

void leds_off(void);
void rainbow_effect(void);
void solid_rgb_effect(void);
void comet_effect(void);
void breathing_effect(void);
void police_effect(void);

void set_color(int inR, int inG, int inB);
};

#endif
