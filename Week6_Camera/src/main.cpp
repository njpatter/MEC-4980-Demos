#include <Arduino.h>
#include <P1AM.h>
#include <MotorEncoder.h>

//Move to observe the Processing Station Turntable, then observe it for 6 seconds
//Move to observe the Sorting Line, then observe it for 4 seconds
//Perform the following 3 times
//Move to observe the Pickup Station, then observe it for 3 seconds
//Move to observe the Warehouse, then observe it for 2 seconds

int modInput = 1;
int modOutput = 2;
int modAnalogIn = 3;

MotorEncoder myFirstMotor(modInput, modOutput, 4, 3, 7);

void setup() {
  delay(1000);
  Serial.begin(9600);
  delay(1000);

  while (!P1.init()) {
    Serial.print("Waiting for connection...");
  }
  Serial.println("Connected!!!");
  
}

void loop() {
  // Move motor to position
  // Wait
  myFirstMotor.MoveCw();
  delay(1000);
  myFirstMotor.MoveCcw();
  delay(1000);
  
}
