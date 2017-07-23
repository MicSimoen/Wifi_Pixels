#include "wifi_pixels_led.h"
#include "parameters.h"
#include "config.h"

#include <FastLED.h>

wifi_pixels_led::wifi_pixels_led(void){
        current_effect = off;
        rainbow_step = 0;
        hue = 0;
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

void wifi_pixels_led::set_hue(uint8_t new_hue){
        hue = new_hue;
}

uint8_t wifi_pixels_led::get_hue(void){
        return hue;
}

void wifi_pixels_led::increment_hue(void){
        hue++;
}

void wifi_pixels_led::set_brightness(uint8_t new_brightness){
        brightness = new_brightness;
}

uint8_t wifi_pixels_led::get_brightness(void){
        return brightness;
}

void wifi_pixels_led::play_current_effect(unsigned long frame_delay){
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
        case breathing:
                breathing_effect();
                break;
        }

        FastLED.delay(frame_delay);
}

void wifi_pixels_led::leds_off(void){
        FastLED.clear ();
        FastLED.show();
}

void wifi_pixels_led::rainbow_effect(void){
        FastLED.setBrightness(brightness);

        for(int dot = 0; dot < NUM_LEDS; dot++) {
                leds[dot] = CHSV(rainbow_step+(dot*NUM_LEDS % 256),255,brightness);
        }
        rainbow_step++;
        rainbow_step = rainbow_step % 256;

        FastLED.show();
}

void wifi_pixels_led::comet_effect(void){
        FastLED.setBrightness(brightness);

        fadeToBlackBy( leds, NUM_LEDS, 20);
        int pos = beatsin16( 13, 0, NUM_LEDS-1 );
        leds[pos] += CHSV( hue, 255, brightness);

        FastLED.show();
}

void wifi_pixels_led::breathing_effect(void){
        for(int dot = 0; dot < NUM_LEDS; dot++) {
                leds[dot] = CHSV((dot*NUM_LEDS % 256),255,brightness);
        }
        float breath = (exp(sin(millis()/2000.0*PI)) - 0.36787944)*108.0;
        FastLED.setBrightness(breath);
        FastLED.show();
}
