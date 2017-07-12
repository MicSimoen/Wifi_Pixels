#include <Arduino.h>
#include <FastLED.h>

   #define NUM_LEDS 16
   #define DATA_PIN 4
   CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
}

void loop() {
  Serial.println("Hello World!");

  for(int dot = 0; dot < NUM_LEDS; dot++) {
            leds[dot] = CRGB::Blue;
            FastLED.show();
            // clear this led for the next time around the loop
            leds[dot] = CRGB::Black;
            delay(30);
        }
}
