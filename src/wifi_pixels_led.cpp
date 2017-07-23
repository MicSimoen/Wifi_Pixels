#include "wifi_pixels_led.h"
#include "parameters.h"
#include "config.h"

#include <FastLED.h>

 wifi_pixels_led::wifi_pixels_led(void){
    current_effect = off;
    rainbow_step = 0;
    comet_hue = 0;
    brightness = STD_BRIGHTNESS;

    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

    // set master brightness control
    FastLED.setBrightness(brightness);
}

void wifi_pixels_led::set_current_effect(led_effect new_current_effect){
    current_effect = new_current_effect;
}
led_effect wifi_pixels_led::get_current_effect(void){
    return current_effect;
}

void wifi_pixels_led::set_comet_hue(uint8_t new_comet_hue){
    comet_hue = new_comet_hue;
}
uint8_t wifi_pixels_led::get_comet_hue(void){
    return comet_hue;
}
void wifi_pixels_led::increment_comet_hue(void){
    comet_hue++;
}

void wifi_pixels_led::set_brightness(uint8_t new_brightness){
    brightness = new_brightness;
}

uint8_t wifi_pixels_led::get_brightness(void){
    return brightness;
}

void wifi_pixels_led::play_current_effect(unsigned long 	frame_delay){
    switch(current_effect) {
    case off:
            leds_off();
            break;
    case rainbow:
            rainbow_effect();
            break;
    case comet:
            comet_effect();
            break;
    }

    FastLED.delay(frame_delay);
}

void wifi_pixels_led::leds_off(void){
        FastLED.clear ();
        FastLED.show();
}

void wifi_pixels_led::rainbow_effect(void){
        for(int dot = 0; dot < NUM_LEDS; dot++) {
                leds[dot] = CHSV(rainbow_step+(dot*NUM_LEDS % 256),255,brightness);
        }
        rainbow_step++;
        rainbow_step = rainbow_step % 256;

        FastLED.show();
}

void wifi_pixels_led::comet_effect(void){
        fadeToBlackBy( leds, NUM_LEDS, 20);
        int pos = beatsin16( 13, 0, NUM_LEDS-1 );
        leds[pos] += CHSV( comet_hue, 255, brightness);

        FastLED.show();
}
