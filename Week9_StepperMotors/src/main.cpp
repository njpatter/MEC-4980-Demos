#include <Arduino.h>
#include "Stepper.h"

Stepper xAxis = Stepper(5, 4, 3, 2, 12);
Stepper yAxis = Stepper(6, 7, 10, 9, 8);

void setup() {
  // put your setup code here, to run once:
  xAxis.begin();
  xAxis.enable(); 
  xAxis.homeBlocking();
  xAxis.moveTo(600);
  yAxis.begin();
  yAxis.enable(); 
  yAxis.homeBlocking();
  yAxis.moveTo(900);
}

void loop() {
  unsigned long nowMicros = micros();
  xAxis.update(nowMicros);
  nowMicros = micros();
  yAxis.update(nowMicros);
  
}
