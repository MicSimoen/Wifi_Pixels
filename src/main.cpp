#include <Arduino.h>
#include <ESP8266Wifi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "config.h"
#include "network_settings.h"
#include "wifi_pixels_led.h"

//--------------------------------------------------
const char* client_id = "WIFIPIXELS"; // Must be unique on the MQTT network
// Topics
const char* light_state_topic = "home/rgb1";
const char* light_set_topic = "home/rgb1/set";

const char* on_cmd = "ON";
const char* off_cmd = "OFF";

const int BUFFER_SIZE = JSON_OBJECT_SIZE(10);

// Maintained state for reporting to HA
byte red = 255;
byte green = 255;
byte blue = 255;
byte brightness = 255;


//Your amount of LEDs to be wrote, and their configuration for FastLED
//

//#define CLOCK_PIN 5
#define CHIPSET     WS2812B
#define COLOR_ORDER GRB

//Onboard blue led
const int txPin = 1;

// Real values to write to the LEDs (ex. including brightness and state)
byte realRed = 0;
byte realGreen = 0;
byte realBlue = 0;

bool stateOn = false;

// Globals for fade/transitions
bool startFade = false;
unsigned long lastLoop = 0;
int transitionTime = 0;
bool inFade = false;
int loopCount = 0;
int stepR, stepG, stepB;
int redVal, grnVal, bluVal;

// Globals for flash
bool flash = false;
bool startFlash = false;
int flashLength = 0;
unsigned long flashStartTime = 0;
byte flashRed = red;
byte flashGreen = green;
byte flashBlue = blue;
byte flashBrightness = brightness;

WiFiClient espClient;
PubSubClient client(espClient);
//----------------------------------------------------

ESP8266WebServer server(80); //Server on port 80

String webPage = "";
wifi_pixels_led led_control;

//------------------------------
// From https://www.arduino.cc/en/Tutorial/ColorCrossfader
/* BELOW THIS LINE IS THE MATH -- YOU SHOULDN'T NEED TO CHANGE THIS FOR THE BASICS
 *
 * The program works like this:
 * Imagine a crossfade that moves the red LED from 0-10,
 *   the green from 0-5, and the blue from 10 to 7, in
 *   ten steps.
 *   We'd want to count the 10 steps and increase or
 *   decrease color values in evenly stepped increments.
 *   Imagine a + indicates raising a value by 1, and a -
 *   equals lowering it. Our 10 step fade would look like:
 *
 *   1 2 3 4 5 6 7 8 9 10
 * R + + + + + + + + + +
 * G   +   +   +   +   +
 * B     -     -     -
 *
 * The red rises from 0 to 10 in ten steps, the green from
 * 0-5 in 5 steps, and the blue falls from 10 to 7 in three steps.
 *
 * In the real program, the color percentages are converted to
 * 0-255 values, and there are 1020 steps (255*4).
 *
 * To figure out how big a step there should be between one up- or
 * down-tick of one of the LED values, we call calculateStep(),
 * which calculates the absolute gap between the start and end values,
 * and then divides that gap by 1020 to determine the size of the step
 * between adjustments in the value.
 */
int calculateStep(int prevValue, int endValue) {
        int step = endValue - prevValue; // What's the overall gap?
        if (step) {                  // If its non-zero,
                step = 1020/step;    //   divide by 1020
        }

        return step;
}

/* The next function is calculateVal. When the loop value, i,
 *  reaches the step size appropriate for one of the
 *  colors, it increases or decreases the value of that color by 1.
 *  (R, G, and B are each calculated separately.)
 */
int calculateVal(int step, int val, int i) {
        if ((step) && i % step == 0) { // If step is non-zero and its time to change a value,
                if (step > 0) {      //   increment the value if step is positive...
                        val += 1;
                }
                else if (step < 0) { //   ...or decrement it if step is negative
                        val -= 1;
                }
        }

        // Defensive driving: make sure val stays in the range 0-255
        if (val > 255) {
                val = 255;
        }
        else if (val < 0) {
                val = 0;
        }

        return val;
}
//_-----------------------

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

void handleBreathing() {
        handleRoot();
        Serial.println("Breathing start!");
        led_control.set_current_effect(breathing);
}

void handlePolice() {
        handleRoot();
        Serial.println("Police start!");
        led_control.set_current_effect(police);
}

void handleSolidRGB() {
        handleRoot();
        Serial.println("Solid RGB start!");
        led_control.set_current_effect(solidRGB);
}

//------------------------------------------------

bool processJson(char* message) {
        StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

        JsonObject& root = jsonBuffer.parseObject(message);

        if (!root.success()) {
                Serial.println("parseObject() failed");
                return false;
        }

        if (root.containsKey("state")) {
                if (strcmp(root["state"], on_cmd) == 0) {
                        stateOn = true;
                }
                else if (strcmp(root["state"], off_cmd) == 0) {
                        stateOn = false;
                }
        }

        // If "flash" is included, treat RGB and brightness differently
        if (root.containsKey("flash")) {
                flashLength = (int)root["flash"] * 1000;

                if (root.containsKey("brightness")) {
                        flashBrightness = root["brightness"];
                }
                else {
                        flashBrightness = brightness;
                }

                if (root.containsKey("color")) {
                        flashRed = root["color"]["r"];
                        flashGreen = root["color"]["g"];
                        flashBlue = root["color"]["b"];
                }
                else {
                        flashRed = red;
                        flashGreen = green;
                        flashBlue = blue;
                }

                flashRed = map(flashRed, 0, 255, 0, flashBrightness);
                flashGreen = map(flashGreen, 0, 255, 0, flashBrightness);
                flashBlue = map(flashBlue, 0, 255, 0, flashBrightness);

                flash = true;
                startFlash = true;
        }
        else { // Not flashing
                flash = false;

                if (root.containsKey("color")) {
                        red = root["color"]["r"];
                        green = root["color"]["g"];
                        blue = root["color"]["b"];
                }

                if (root.containsKey("brightness")) {
                        brightness = root["brightness"];
                }

                if (root.containsKey("transition")) {
                        transitionTime = root["transition"];
                }
                else {
                        transitionTime = 0;
                }
        }

        return true;
}

void sendState() {
        StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

        JsonObject& root = jsonBuffer.createObject();

        root["state"] = (stateOn) ? on_cmd : off_cmd;
        JsonObject& color = root.createNestedObject("color");
        color["r"] = red;
        color["g"] = green;
        color["b"] = blue;

        root["brightness"] = brightness;

        char buffer[root.measureLength() + 1];
        root.printTo(buffer, sizeof(buffer));

        client.publish(light_state_topic, buffer, true);
}

void callback(char* topic, byte* payload, unsigned int length) {
        Serial.print("Message arrived [");
        Serial.print(topic);
        Serial.print("] ");

        char message[length + 1];
        for (int i = 0; i < length; i++) {
                message[i] = (char)payload[i];
        }
        message[length] = '\0';
        Serial.println(message);

        if (!processJson(message)) {
                return;
        }

        if (stateOn) {
                // Update lights
                realRed = map(red, 0, 255, 0, brightness);
                realGreen = map(green, 0, 255, 0, brightness);
                realBlue = map(blue, 0, 255, 0, brightness);
        }
        else {
                realRed = 0;
                realGreen = 0;
                realBlue = 0;
        }

        startFade = true;
        inFade = false; // Kill the current fade

        sendState();
}

void reconnect() {
        // Loop until we're reconnected
        while (!client.connected()) {
                Serial.print("Attempting MQTT connection...");
                // Attempt to connect
                if (client.connect(client_id, mqtt_username, mqtt_password)) {
                        Serial.println("connected");
                        client.subscribe(light_set_topic);
                } else {
                        Serial.print("failed, rc=");
                        Serial.print(client.state());
                        Serial.println(" try again in 5 seconds");
                        // Wait 5 seconds before retrying
                        delay(5000);
                }
        }
}
//-----------------------------------------------


void setup_wifi(void) {
        delay(10);

        Serial.print("INFO: WiFi connecting to: ");
        Serial.println(SSID);

        WiFi.mode(WIFI_STA);
        WiFi.begin(SSID, PSWD);

        while (WiFi.waitForConnectResult() != WL_CONNECTED) {
                Serial.println("ERROR: Connection Failed! Rebooting...");
                delay(5000);
                ESP.restart();
        }

        Serial.println(F("INFO: WiFi connected"));
        Serial.print("INFO: IP address: ");
        Serial.println(WiFi.localIP());
        Serial.print("MAC Addr: ");
        Serial.println(WiFi.macAddress());
}

void setup_OTA(void){
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
}

void setup_mqtt(void){
        client.setServer(mqtt_server, mqtt_port);
        client.setCallback(callback);
}

void setup() {
        webPage += "<h1>Wifi Pixels Control</h1>";
        webPage += "<p><a href=\"LedsOff\"><button>OFF</button></a></p>";
        webPage += "<p><a href=\"RainbowOn\"><button>Rainbow</button></a></p>";
        webPage += "<p><a href=\"CometOn\"><button>Comet</button></a></p>";
        webPage += "<p><a href=\"BreathingOn\"><button>Breathing</button></a></p>";
        webPage += "<p><a href=\"PoliceOn\"><button>Police</button></a></p>";
        webPage += "<p><a href=\"SolidRGB\"><button>Solid RGB</button></a></p>";

        #if defined(DEBUG_SERIAL)
        Serial.begin(115200);
        #elif defined(DEBUG_TELNET)
        telnetServer.begin();
        telnetServer.setNoDelay(true);
        #endif

        Serial.println("");

        Serial.println("Booting");

        setup_wifi();

        //setup_mqtt();

        setup_OTA();

        server.on("/", handleRoot);
        server.on("/LedsOff", handleLedsOff);
        server.on("/RainbowOn", handleRainbow);
        server.on("/CometOn", handleComet);
        server.on("/BreathingOn", handleBreathing);
        server.on("/PoliceOn", handlePolice);
        server.on("/SolidRGB", handleSolidRGB);

        server.begin();                  //Start server
        Serial.println("HTTP server started");
}

void loop() {
        ArduinoOTA.handle();

        server.handleClient();          //Handle client requests
        led_control.play_current_effect(1000/STD_FPS);

//---------------------------------------------

        // if (!client.connected()) {
        //         reconnect();
        // }
        // client.loop();

        if (flash) {
                if (startFlash) {
                        startFlash = false;
                        flashStartTime = millis();
                }

                if ((millis() - flashStartTime) <= flashLength) {
                        if ((millis() - flashStartTime) % 1000 <= 500) {
                                led_control.set_color(flashRed, flashGreen, flashBlue);
                        }
                        else {
                                led_control.set_color(0, 0, 0);
                                // If you'd prefer the flashing to happen "on top of"
                                // the current color, uncomment the next line.
                                // led_control.set_color(realRed, realGreen, realBlue);
                        }
                }
                else {
                        flash = false;
                        led_control.set_color(realRed, realGreen, realBlue);
                }
        }

        if (startFade) {
                // If we don't want to fade, skip it.
                if (transitionTime == 0) {
                        led_control.set_color(realRed, realGreen, realBlue);

                        redVal = realRed;
                        grnVal = realGreen;
                        bluVal = realBlue;

                        startFade = false;
                }
                else {
                        loopCount = 0;
                        stepR = calculateStep(redVal, realRed);
                        stepG = calculateStep(grnVal, realGreen);
                        stepB = calculateStep(bluVal, realBlue);

                        inFade = true;
                }
        }

        if (inFade) {
                startFade = false;
                unsigned long now = millis();
                if (now - lastLoop > transitionTime) {
                        if (loopCount <= 1020) {
                                lastLoop = now;

                                redVal = calculateVal(stepR, redVal, loopCount);
                                grnVal = calculateVal(stepG, grnVal, loopCount);
                                bluVal = calculateVal(stepB, bluVal, loopCount);

                                led_control.set_color(redVal, grnVal, bluVal); // Write current values to LED pins

                                Serial.print("Loop count: ");
                                Serial.println(loopCount);
                                loopCount++;
                        }
                        else {
                                inFade = false;
                        }
                }
        }

//-------------------------------

        EVERY_N_MILLISECONDS( 100 ) {
                led_control.increment_hue();
        }
}
