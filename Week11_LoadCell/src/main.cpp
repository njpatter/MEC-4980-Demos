#include <Arduino.h> 
#include <Wire.h>
#include "SparkFun_Qwiic_Scale_NAU7802_Arduino_Library.h"

NAU7802 loadcell;

int sampleRate = 10;
int minVal = 161000;
int minMass = -400;
int maxVal = -167000;
int maxMass = 400;
int zeroVal = -8000;

int readingToGrams(long val) {
  return float(maxMass - minMass) / float(maxVal - minVal) * val;
}

void setup() { 
  delay(1000);
  Serial.begin(9600);
  Wire.begin();
  while(!loadcell.begin()) {
    Serial.println("Waiting for load cell to start");
    delay(100);
  }
  loadcell.setSampleRate(sampleRate);
  loadcell.setGain(128);
  loadcell.calibrateAFE();
  delay(500);
  loadcell.calculateZeroOffset(50); 
}

void loop() { 
  if(loadcell.available()) {
    int32_t lcReading = loadcell.getReading() - loadcell.getZeroOffset();
    Serial.print(lcReading);
    Serial.print(" means g = ");
    Serial.print(readingToGrams(lcReading));
    Serial.print(" at time ");
    Serial.println(millis());
  }
  delay(1000/sampleRate);
} 