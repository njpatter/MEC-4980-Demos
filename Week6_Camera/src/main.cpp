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

int turnPos[] = {25, 80, 160, 240};
int tiltPos[] = {30, 45, 60, 60};
int obsDelay[] = {3000, 6000, 3000, 3000};
int currentPos = 0;

// MotorEncoder(int mInput, int mOutput, int pCw, int pCcw, int pE, int sw )
MotorEncoder myFirstMotor(modInput, modOutput, 4, 3, 7, 2);
MotorEncoder tiltMotor(modInput, modOutput, 1, 2, 5, 1);

void setup() {
  delay(1000);
  Serial.begin(9600);
  delay(1000);

  while (!P1.init()) {
    Serial.print("Waiting for connection...");
  }
  Serial.println("Connected!!!");
  myFirstMotor.Home();
  tiltMotor.Home();
}

void loop() { 
  bool doneMoving = myFirstMotor.MoveTo(turnPos[currentPos]) & tiltMotor.MoveTo(tiltPos[currentPos]);
  if (doneMoving) {
    delay(obsDelay[currentPos]);
    currentPos = (currentPos + 1) % 4; 
  } 
}
