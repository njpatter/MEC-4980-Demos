#include <Arduino.h>

// put function declarations here:
int myFunction(int, int);

float vdd = 3.3;
float R25 = 100000.0;
float T25 = 298.0;
float beta = 3750.0;
long prevTime = 0;
volatile float globalTemp = -1.0;

hw_timer_t *tempTimer = NULL;

float intToVoltage(int reading) {
  return float(reading) / 4095.0 * 3.3;
}

float voltageToResistance (float vout) {
  return (vout* 22000) / (vdd - vout);
}

float resistanceToTemp (float R) {
  return 1.0 / ((1.0 / T25) + 1.0/beta * log(R/R25)); 
}

void getTemp() { 
  float voltage = intToVoltage(analogRead(A0));
  float resistance = voltageToResistance(voltage);
  globalTemp = resistanceToTemp(resistance);
}

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
  Serial.begin(115200);

  tempTimer = timerBegin(0, 80, true);
  timerAttachInterrupt(tempTimer, &getTemp, true);
  timerAlarmWrite(tempTimer, 1000000, true);
  timerAlarmEnable(tempTimer);
}

void loop() {
  if (millis() - prevTime > 1000) {
    timerAlarmDisable(tempTimer);
    Serial.print("Temp is: ");
    Serial.print(globalTemp);
    Serial.println(" degrees C");
    timerAlarmWrite(tempTimer, 1000000, true);
    timerAlarmEnable(tempTimer);
    prevTime = millis();
  }  
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}