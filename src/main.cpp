#include <Arduino.h>
#include <ESP8266Wifi.h>
#include <ESP8266WebServer.h>

#include "config.h"
#include "wifi_settings.h"
#include "wifi_pixels_led.h"

ESP8266WebServer server(80); //Server on port 80

String webPage = "";
wifi_pixels_led led_control;

void handleRoot() {
        server.send(200, "text/html", webPage);
}

void handleLedsOff() {
        handleRoot();
        Serial.println("OFF!");
        led_control.set_current_effect(off);
}

void handleRainbow() {
        handleRoot();
        Serial.println("Rainbow start!");
        led_control.set_current_effect(rainbow);
}

void handleComet() {
        handleRoot();
        Serial.println("Comet start!");
        led_control.set_current_effect(comet);
}

void setup() {
        webPage += "<h1>Wifi Pixels Control</h1>";
        webPage += "<p><a href=\"LedsOff\"><button>OFF</button></a></p>";
        webPage += "<p><a href=\"RainbowOn\"><button>Rainbow</button></a></p>";
        webPage += "<p><a href=\"CometOn\"><button>Comet</button></a></p>";
        Serial.begin(9600);

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
        led_control.play_current_effect(1000/STD_FPS);

        EVERY_N_MILLISECONDS( 100 ) {
                led_control.increment_comet_hue();
        }
}
