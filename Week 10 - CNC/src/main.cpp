#include <Arduino.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include "GCodeParser.h"

AccelStepper xAxis(AccelStepper::DRIVER, 5, 4);
AccelStepper yAxis(AccelStepper::DRIVER, 6, 7);

MultiStepper cnc;

int xMin = 3;
int yMin = 10;

int target = 500;
int velocity = 800;
int accel = 800;
int accel_inc = 500;

// Track target positions in steps (absolute)
long curX = 0;
long curY = 0;

// Serial input buffer
String inputLine;

void Home() {
  xAxis.moveTo(-10000);
  yAxis.moveTo(-10000);
  Serial.println("Starting home routine");
  while (digitalRead(xMin) || digitalRead(yMin)) {
    if (digitalRead(xMin)) {
      xAxis.run();
    }
    if (digitalRead(yMin)) {
      yAxis.run();
    }
    Serial.println("Homing...");
    
  }
  Serial.println("Done homing");
  xAxis.setCurrentPosition(0);
  xAxis.moveTo(0);
  yAxis.setCurrentPosition(0);
  yAxis.moveTo(0);
}

void Rapid(int xPos, int yPos) {
  xAxis.moveTo(xPos);
  yAxis.moveTo(yPos);
  // Use bitwise OR to ensure both run() calls execute
  while (xAxis.distanceToGo() != 0 || yAxis.distanceToGo() != 0) {
    xAxis.run();
    yAxis.run();
  }
}

void Linear(int xPos, int yPos) {
  long t[] = {xPos, yPos};
  cnc.moveTo(t);
  cnc.runSpeedToPosition();
  //while () {
  //  Serial.println("Moving in a linear fashion");
  //}
}

static void processGCodeLine(const String &line) {
  GCodeParser::Command cmd = GCodeParser::parseLine(line);
  if (!cmd.valid) {
    Serial.print(F("ERR: "));
    Serial.println(cmd.error);
    return;
  }

  switch (cmd.type) {
    case GCodeParser::TYPE_G28: {
      Serial.println(F("CMD: G28 (Home)"));
      Home();
      curX = 0;
      curY = 0;
      break;
    }
    case GCodeParser::TYPE_G0: {
      long tx = curX;
      long ty = curY;
      if (cmd.hasX) tx = (long)(cmd.x + 0.5); // Round to nearest integer
      if (cmd.hasY) ty = (long)(cmd.y + 0.5); // Round to nearest integer
      Serial.print(F("CMD: G0 X")); Serial.print(tx); Serial.print(F(" Y")); Serial.println(ty);
      Rapid(tx, ty);
      // Update current position from actual stepper positions
      curX = xAxis.currentPosition();
      curY = yAxis.currentPosition();
      Serial.print(F("Pos: X")); Serial.print(curX); Serial.print(F(" Y")); Serial.println(curY);
      break;
    }
    case GCodeParser::TYPE_G1: {
      long tx = curX;
      long ty = curY;
      if (cmd.hasX) tx = (long)(cmd.x + 0.5); // Round to nearest integer
      if (cmd.hasY) ty = (long)(cmd.y + 0.5); // Round to nearest integer
      if (cmd.hasF) {
        xAxis.setMaxSpeed((float)cmd.f);
        yAxis.setMaxSpeed((float)cmd.f);
      }
      Serial.print(F("CMD: G1 X")); Serial.print(tx); Serial.print(F(" Y")); Serial.println(ty);
      Linear(tx, ty);
      // Update current position from actual stepper positions
      curX = xAxis.currentPosition();
      curY = yAxis.currentPosition();
      Serial.print(F("Pos: X")); Serial.print(curX); Serial.print(F(" Y")); Serial.println(curY);
      break;
    }
    case GCodeParser::TYPE_M3: {
      Serial.print(F("CMD: M3"));
      if (cmd.hasS) { Serial.print(F(" S")); Serial.print(cmd.s); }
      Serial.println();
      // TODO: hook spindle control if available
      break;
    }
    default:
      Serial.println(F("ERR: Unsupported/unknown command type"));
      break;
  }
}

void setup() {
  Serial.begin(9600);
  delay(200);
  Serial.println(F("CNC Controller Ready"));
  pinMode(xMin, INPUT_PULLUP);
  pinMode(yMin, INPUT_PULLUP);
  xAxis.setMaxSpeed(velocity);
  xAxis.setAcceleration(accel); 
  yAxis.setMaxSpeed(velocity);
  yAxis.setAcceleration(accel);
  Home();
  cnc.addStepper(xAxis);
  cnc.addStepper(yAxis);
  
}

void loop() {
  // Read incoming serial data line-by-line
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\r') continue; // ignore CR
    if (c == '\n') {
      inputLine.trim();
      if (inputLine.length() > 0) {
        Serial.print(F(">> "));
        Serial.println(inputLine);
        processGCodeLine(inputLine);
        Serial.println(F("OK")); // Send OK after completing command
      }
      inputLine = "";
    } else {
      // Only add printable characters
      if (c >= 32 && c <= 126) {
        inputLine += c;
      }
      if (inputLine.length() > 120) {
        Serial.println(F("ERR: Line too long"));
        inputLine = "";
      }
    }
  }
}
