#include <Arduino.h>
#include <ESP8266Wifi.h>
#include <ESP8266WebServer.h>
#include <FastLED.h>

#include "config.h"
#include "config_override.h" //Included after config.h to override some settings

CRGB leds[NUM_LEDS];
int effect = 0;
uint8_t rainBowStep = 0; //used by rainbow
uint8_t gHue = 0; // rotating "base color" used by comet

ESP8266WebServer server(80); //Server on port 80

String webPage = "";

void handleRoot() {
        server.send(200, "text/html", webPage);
}

void handleLedsOff() {
        handleRoot();
        Serial.println("OFF!");
        effect = 0;
}

void handleRainbow() {
        handleRoot();
        Serial.println("Rainbow start!");
        effect = 1;
}

void handleComet() {
        handleRoot();
        Serial.println("Comet start!");
        effect = 2;
}

void ledsOff(){
        FastLED.clear ();
}

void rainBow(){
        for(int dot = 0; dot < NUM_LEDS; dot++) {
                leds[dot] = CHSV(rainBowStep+(dot*NUM_LEDS % 256),255,BRIGHTNESS);
        }
        rainBowStep++;
        rainBowStep = rainBowStep % 256;
}

void Comet(){
        fadeToBlackBy( leds, NUM_LEDS, 20);
        int pos = beatsin16( 13, 0, NUM_LEDS-1 );
        leds[pos] += CHSV( gHue, 255, BRIGHTNESS);
}

void setup() {
        webPage += "<h1>Wifi Pixels Control</h1>";
        webPage += "<p><a href=\"LedsOff\"><button>OFF</button></a></p>";
        webPage += "<p><a href=\"RainbowOn\"><button>Rainbow</button></a></p>";
        webPage += "<p><a href=\"CometOn\"><button>Comet</button></a></p>";
        Serial.begin(9600);
        FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
        // set master brightness control
        FastLED.setBrightness(BRIGHTNESS);

        WiFi.begin(SSID, PSWD);

        Serial.print("Connecting");
        while (WiFi.status() != WL_CONNECTED)
        {
                delay(500);
                Serial.print(".");
        }
        Serial.println();
        Serial.print("Connected, IP address: ");
        Serial.println(WiFi.localIP());

        server.on("/", handleRoot);
        server.on("/LedsOff", handleLedsOff);
        server.on("/RainbowOn", handleRainbow);
        server.on("/CometOn", handleComet);

        server.begin();                  //Start server
        Serial.println("HTTP server started");
}

void loop() {
        server.handleClient();          //Handle client requests

        switch(effect) {
        case 0:
                ledsOff();
                break;
        case 1:
                rainBow();
                break;
        case 2:
                Comet();
                break;
        }
        FastLED.show();
        FastLED.delay(1000/FPS);

        EVERY_N_MILLISECONDS( 100 ) {
                gHue++;
        }
}
