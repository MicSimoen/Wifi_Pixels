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
  Serial.println("Rainbow start!");
  for(int step = 0; step < 256; step++){
    for(int dot = 0; dot < NUM_LEDS; dot++) {
      leds[dot] = CHSV(step+(dot*NUM_LEDS % 256),255,63);
      //FastLED.show();
      //delay(30);
    }
    FastLED.show();
    delay(12);
  }
}
