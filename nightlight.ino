// This #include statement was automatically added by the Particle IDE.
#include <MQTT.h>

// This #include statement was automatically added by the Particle IDE.
#include <neopixel.h>

#include "application.h"
#include "math.h"


// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_COUNT 12
#define PIXEL_PIN D0
#define PIXEL_TYPE WS2812B

int delayval = 50; 
float MaximumBrightness = 255;
float SpeedFactor = 0.008; // This controls the speed. This is just a guess, but it looks OK
int stepDelay = 5;
int INTENSITY = 150;

MQTT client("<server ip / hostname>", 1883, callback);


Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

void setup() {
    Particle.function("led",ledToggle);
    strip.begin();
    strip.setBrightness(85);
    
    // connect to the server
    client.connect("sparkclient");
    
    // publish/subscribe
    // These mqtt topics can be anything you like, just keep them consistent
    if (client.isConnected()) {
        client.publish("/nightlight/status","ON");
        client.subscribe("/nightlight/command");
        client.subscribe("/nightlight/status");
        client.subscribe("/nightlight/brightness/status");
        client.subscribe("/nightlight/brightness");
        client.subscribe("/nightlight/rgb/status");
        client.subscribe("/nightlight/rgb/set");
    }
}

void loop() {
    if (client.isConnected()) {
        client.loop();
    }
}

void colourWipe(uint32_t r, uint32_t g, uint32_t b) {
  for(uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
    strip.show();
    delay(delayval);
  }
}

void breathe(uint32_t r, uint32_t g, uint32_t b) {
    for (int i = 0; i < 65535; i++) {
        // Intensity will go from 10 - MaximumBrightness in a "breathing" manner
        float intensity = MaximumBrightness /2.0 * (1.0 + sin(SpeedFactor * i));
        strip.setBrightness(intensity);
        // Now set every LED to that color
        for (int ledNumber = 0; ledNumber < PIXEL_COUNT; ledNumber++) {
            strip.setPixelColor(ledNumber, r, g, b);
        }
        strip.show();
        delay(stepDelay);
    }
}

int ledToggle(String command) {
    if (command=="green") {
        strip.setBrightness(INTENSITY);
        colourWipe(0, 255, 0);
        //breathe(0, 255, 0);
        return 0;
    }
    else if (command=="blue") {
        strip.setBrightness(INTENSITY);
        colourWipe(0, 0, 255);
        //breathe(0, 0, 255);
        return 1;
    }
    else if(command =="red") {
        strip.setBrightness(INTENSITY);
        colourWipe(255,0,0);
        //breathe(255, 0, 0);
        return 2;
    }
    else if(command =="yellow") {
        strip.setBrightness(INTENSITY);
        colourWipe(255, 255, 0);
        //breathe(255, 255, 0);
        return 3;
    }
    else if(command == "pink") {
        strip.setBrightness(INTENSITY);
        colourWipe(255, 0, 255);
        //breathe(255, 0, 255);
        return 4;
    }
    else {
        return -1;
    }
}
    
// recieve message
void callback(char* topic, byte* payload, unsigned int length) {
    char p[length + 1];
    memcpy(p, payload, length);
    p[length] = NULL;
    String message(p);
    
    // do the switch for the multiple topics
    if (strcmp(topic,"/nightlight/command")==0) {
        if(strcmp(message, "OFF")==0) {
            strip.setBrightness(0);
            strip.show();
            client.publish("/nightlight/status", "OFF");
        }
        else if(strcmp(message, "ON")==0) {
            strip.setBrightness(150);
            strip.show();
            client.publish("/nightlight/status", "ON");
        }
    }
    else if (strcmp(topic,"/nightlight/brightness")==0) {
        client.publish("/nightlight/outTopic", message);
    }
    else if (strcmp(topic,"/nightlight/rgb/set")==0) {
        char * params = new char[message.length() + 1];
        strcpy(params, message.c_str());
        char * q = strtok(params, ",");
        
        int commandStep = 0;
        int red = 0;
        int green = 0;
        int blue = 0;
        
        while (q != NULL) {
            if (commandStep == 0) {
                red = atoi(q);
            }
            else if (commandStep == 1) {
                green = atoi(q);
            }
            else if (commandStep == 2) {
                blue = atoi(q);
            }

            commandStep++;
            q = strtok(NULL, ",");
        }
        colourWipe(red, green, blue);
        client.publish("/nightlight/rgb/status", message);
    }
}
