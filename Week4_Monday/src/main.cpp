#include <Arduino.h>

int buttonCounter = 0;
bool prevPressed = false;
int switchPin = 10;
unsigned long debounceDelay = 50;
unsigned long prevTime = 0;

void buttonPress() {
  unsigned long currentTime = millis();
  if (currentTime - prevTime > debounceDelay) {
    buttonCounter++;
    prevTime = currentTime;
  }
}

void setup() {
  Serial.begin(9600);
  delay(2000);
  pinMode(switchPin, INPUT_PULLDOWN);
  attachInterrupt(switchPin, buttonPress, CHANGE);
}

void loop() {
  Serial.print("Button tally: ");
  Serial.println(buttonCounter); 
  delay(1000);
  
}