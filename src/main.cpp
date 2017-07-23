#include <Arduino.h>
#include <ESP8266Wifi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
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
        Serial.begin(115200);
        Serial.println("");

        Serial.println("Booting");
        WiFi.mode(WIFI_STA);
        WiFi.begin(SSID, PSWD);
        Serial.print("Connecting to: ");
        Serial.println(SSID);

        while (WiFi.waitForConnectResult() != WL_CONNECTED) {
                Serial.println("Connection Failed! Rebooting...");
                delay(5000);
                ESP.restart();
        }

        Serial.println("Connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("MAC Addr: ");
        Serial.println(WiFi.macAddress());

        //----------------------------------------------------------------------
        //OTA Routines

        // Port defaults to 8266
        // ArduinoOTA.setPort(8266);

        // Hostname defaults to esp8266-[ChipID]
        // ArduinoOTA.setHostname("myesp8266");

        // No authentication by default
        // ArduinoOTA.setPassword((const char *)"123");

        ArduinoOTA.onStart([]() {
                Serial.println("Start");
        });
        ArduinoOTA.onEnd([]() {
                Serial.println("\nEnd");
        });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
                Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
        });
        ArduinoOTA.onError([](ota_error_t error) {
                Serial.printf("Error[%u]: ", error);
                if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
                else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
                else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
                else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
                else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });
        ArduinoOTA.begin();

        //----------------------------------------------------------------------

        server.on("/", handleRoot);
        server.on("/LedsOff", handleLedsOff);
        server.on("/RainbowOn", handleRainbow);
        server.on("/CometOn", handleComet);

        server.begin();                  //Start server
        Serial.println("HTTP server started");
}

void loop() {
        ArduinoOTA.handle();

        server.handleClient();          //Handle client requests
        led_control.play_current_effect(1000/STD_FPS);

        EVERY_N_MILLISECONDS( 100 ) {
                led_control.increment_comet_hue();
        }
}
