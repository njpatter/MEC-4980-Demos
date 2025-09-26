#include <Arduino.h> 
#include <Wire.h> 
#include <stdint.h> 
#include <res/qw_fnt_5x7.h> 
#include <math.h> 

volatile int buttonCounter = 0;
bool prevPressed = false;
int switchPin = 10;
unsigned long debounceDelay = 50;
unsigned long doublePressTime = 500;
unsigned long prevTime = 0; 

enum PressType {
  NoPress, // 0
  SinglePress, //1 
  DoublePress, //2
  LongPress //3
};

enum MachineState {
  OffState,
  TwoAxis,
  XAxis,
  YAxis,
  RawData,
  StateLength
};

volatile PressType currentPress = NoPress;
volatile MachineState currentState = OffState;

void buttonPress() {
  unsigned long currentTime = millis();
  if (currentTime - prevTime > debounceDelay) {
    if (currentTime - prevTime < doublePressTime) {
      currentPress = DoublePress;
    } else {
      currentPress = SinglePress;
    }
    prevTime = currentTime;

  }
}

void setup() {
  delay(1000);
  Serial.begin(9600); 
  while (!Serial) {
    yield();
  } 
  pinMode(switchPin, INPUT_PULLDOWN);
  attachInterrupt(switchPin, buttonPress, RISING); 
}

void loop() {
  if (currentPress == DoublePress ) { //&& currentState != OffState
    currentState = (MachineState)(((int)currentState + 1) % (int)StateLength);
    currentState = (MachineState)max((int)currentState, 1);
  }
  if (currentPress != NoPress) {
    Serial.print("Current State type: ");
    Serial.println((int)currentState);
    currentPress = NoPress;
  }  

  switch (currentState)
  {
  case OffState:
    /* code */
    break;
  case TwoAxis: 

    break;
  default:
    break;
  }
   
}