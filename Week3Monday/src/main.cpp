// Example-05_ScrollFlip.ino
// 
// This is a library written for SparkFun Qwiic OLED boards that use the SSD1306.
//
// SparkFun sells these at its website: www.sparkfun.com
//
// Do you like this library? Help support SparkFun. Buy a board!
//
//   Micro OLED             https://www.sparkfun.com/products/14532
//   Transparent OLED       https://www.sparkfun.com/products/15173
//   "Narrow" OLED          https://www.sparkfun.com/products/17153
// 
// 
// Written by Kirk Benell @ SparkFun Electronics, March 2022
//
// This library configures and draws graphics to OLED boards that use the 
// SSD1306 display hardware. The library only supports I2C.
// 
// Repository:
//     https://github.com/sparkfun/SparkFun_Qwiic_OLED_Arduino_Library
//
// Documentation:
//     https://sparkfun.github.io/SparkFun_Qwiic_OLED_Arduino_Library/
//
//
// SparkFun code, firmware, and software is released under the MIT License(http://opensource.org/licenses/MIT).
//
// SPDX-License-Identifier: MIT
//
//    The MIT License (MIT)
//
//    Copyright (c) 2022 SparkFun Electronics
//    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
//    associated documentation files (the "Software"), to deal in the Software without restriction,
//    including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
//    and/or sell copies of the Software, and to permit persons to whom the Software is furnished to
//    do so, subject to the following conditions:
//    The above copyright notice and this permission notice shall be included in all copies or substantial
//    portions of the Software.
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//    NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
//    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//////////////////////////////////////////////////////////////////////////////////////////
// Example 5 for the SparkFun Qwiic OLED Arduino Library
//
// >> Overview <<
//
// This demo shows the various display options - scrolling, flipping, invert.
//
// NOTE: Scrolling isn't supported on the Transparent OLED
//
//////////////////////////////////////////////////////////////////////////////////////////
//
// >>> SELECT THE CONNECTED DEVICE FOR THIS EXAMPLE <<<
//
// The Library supports three different types of SparkFun boards. The demo uses the following
// defines to determine which device is being used. Uncomment the device being used for this demo.
//
// The default is Micro OLED

#define MICRO
//#define NARROW
//#define TRANSPARENT

//////////////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Include the SparkFun qwiic OLED Library
#include <SparkFun_Qwiic_OLED.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;

#if defined(TRANSPARENT)
QwiicTransparentOLED myOLED;
const char * deviceName = "Transparent OLED";

#elif defined(NARROW)
QwiicNarrowOLED myOLED;
const char * deviceName = "Narrow OLED";

#else
QwiicMicroOLED myOLED;
const char * deviceName = "Micro OLED";

#endif

int yoffset;
float targetTemperature = 20.0;
char degreeSys[] = "C";
int pinButton = 10;
int pinUp = 11;
int pinDown = 12;
bool prevPressed = false;
bool prevUp = false;
bool prevDown = false;

enum MachineStates {
    DisplayTemps, // 0
    SetTemp, // 1
    ChooseSystem // 2
};

MachineStates currentState;

////////////////////////////////////////////////////////////////////////////////////////////////
// setup()
// 
// Standard Arduino setup routine

void setup()
{
    currentState = DisplayTemps;
    pinMode(pinButton, INPUT_PULLDOWN);
    pinMode(pinUp, INPUT_PULLDOWN);
    pinMode(pinDown, INPUT_PULLDOWN);

    Serial.begin(9600);
    delay(3000);
    Serial.println("Testing BME sensor");
    if (! bme.begin(0x77, &Wire)) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }

    delay(500);   //Give display time to power on

    Serial.println("\n\r-----------------------------------");

    Serial.print("Running Test #5 on: ");
    Serial.println(String(deviceName));

    if(!myOLED.begin()){

        Serial.println("- Device Begin Failed");
        while(1);
    }

    yoffset = (myOLED.getHeight() - myOLED.getFont()->height)/2;

    delay(1000);
}

// Our testing functions

void scroll_right(void){

    myOLED.scrollStop();
    myOLED.scrollRight(0, 7, SCROLL_INTERVAL_2_FRAMES); 
}

void scroll_right_vert(void){
    myOLED.scrollStop();    
    myOLED.scrollVertRight(0, 7, SCROLL_INTERVAL_3_FRAMES); 
}

void scroll_left(void){
    myOLED.scrollStop();    
    myOLED.scrollLeft(0, 7, SCROLL_INTERVAL_4_FRAMES);
}

void scroll_left_vert(void){
    myOLED.scrollStop();    
    myOLED.scrollVertLeft(0, 7, SCROLL_INTERVAL_5_FRAMES);
}

void scroll_stop(void){
    myOLED.scrollStop();
}

void flip_horz(void){

    for(int i=0; i < 6; i++){
        myOLED.flipHorizontal(!(i & 0x01));
        delay(800);
    }
}

void flip_vert(void){
    for(int i=0; i < 6; i++){
        myOLED.flipVertical(!(i & 0x01));
        delay(800);
    }
}

void invert(void){
    for(int i=0; i < 6; i++){
        myOLED.invert(!(i & 0x01));
        delay(800);
    }    
}

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Use an array of testing functions, with a title, to run the tests

typedef void (*testFn)(void);
typedef struct _testRoutines{
    void (*testFn)(void);
    const char *title;
}testRoutine;

static const testRoutine testFunctions[] = {
    {scroll_right, "Right>"},
    {scroll_right_vert, "^Right-Up>"},
    {scroll_left, "<Left"},
    {scroll_left_vert, "<Left-Up^"},
    {scroll_stop, "<STOP>"},
    {flip_horz, "-Flip-Horz-"},    
    {flip_vert, "|Flip-Vert|"},    
    {invert, "**INVERT**"}        
}; 


float cToF(float degC) {
    return (degC +32.0) * 9.0 / 5.0;
}


void loop()
{
    if(digitalRead(pinButton) && !prevPressed) {
        currentState = MachineStates(((int)currentState + 1) % 3);
    }

    
    prevPressed = digitalRead(pinButton);

    char myNewText[50];
    if (currentState == DisplayTemps) {
        float temp = bme.readTemperature();
        
        sprintf(myNewText, "Tc: %.1f ", temp );
        myOLED.erase();
        myOLED.text(3, yoffset, myNewText);

        sprintf(myNewText, "Ttar: %.1f", targetTemperature);
        myOLED.text(3, yoffset + 12, myNewText);
        myOLED.display(); 
    } else if (currentState == SetTemp) {
        if (digitalRead(pinUp) && !prevUp) {
            targetTemperature++; // targetTemperature += 1.0; // targetTemperature = targerTempreature + 1.0;
        }
        if (digitalRead(pinDown) && !prevDown) {
            targetTemperature--;
        }
        prevUp = digitalRead(pinUp);
        prevDown = digitalRead(pinDown);
        sprintf(myNewText, "Ttar: %.1f", targetTemperature);
        myOLED.erase();
        myOLED.text(3, yoffset, myNewText);
        myOLED.display(); 
    } else if (currentState == ChooseSystem) {
        sprintf(myNewText, "System: %s", degreeSys); 
        myOLED.erase();
        myOLED.text(3, yoffset, myNewText);
        myOLED.display(); 
    }

    
}