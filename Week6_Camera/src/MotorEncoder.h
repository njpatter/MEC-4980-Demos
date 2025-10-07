#ifndef MOTOR_ENCODER_H
#define MOTOR_ENCODER_H
#endif

#include <Arduino.h>
#include <P1AM.h>

class MotorEncoder {
private:
    int modInput;
    int modOutput;
    int pinCw;
    int pinCcw;
    int pinEncoder;
    int pulseCount;
    bool prevState;
    int dir;

public:
    MotorEncoder(int mInput, int mOutput, int pCw, int pCcw, int pE ): modInput(mInput), modOutput(mOutput), pinCw(pCw), pinCcw(pCcw), pinEncoder(pE), pulseCount(0), prevState(false), dir(1) {;}

    void begin() {
        //instance = this;
    }

    void MoveCw () {
        P1.writeDiscrete(true, modOutput, pinCw);
        P1.writeDiscrete(false, modOutput, pinCcw);
        dir = -1;
    }

    void MoveCcw () {
        P1.writeDiscrete(true, modOutput, pinCcw);
        P1.writeDiscrete(false, modOutput, pinCw);
        dir = 1;
    }

    void Stop() {
        P1.writeDiscrete(false, modOutput, pinCcw);
        P1.writeDiscrete(false, modOutput, pinCw);
    }

    void UpdatePulse() {
        bool currentState = P1.readDiscrete(modInput, pinEncoder);
        if (currentState && !prevState) {
            pulseCount += dir;
        }
        prevState = currentState;
    }

    void ZeroPulse() {
        pulseCount = 0;
    }

    bool MoveTo(int targetPos) {
        if (pulseCount < targetPos) {
            MoveCw();
        } else if (pulseCount > targetPos) {
            MoveCcw();
        } else {
            Stop();
            return true;
        }
    }
};