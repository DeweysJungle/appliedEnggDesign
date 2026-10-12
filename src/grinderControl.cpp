#include <Arduino.h>
#include <grinderControl.hpp>
#include <servo.h>

const int srgPin = A0;  //pin reading voltage divider for photoresistor
const int servoPin = 46; //Pin attached to signal of the servo


//Create Instance of servo class called s1
Servo s1;


bool checkStorage(){            //Check if enough beans are available for next cup
    int avgLight = 0;
    for(int i = 0;i < 50;i++){  //Take the avg of 50 readings
        avgLight += analogRead(srgPin);
    }
    avgLight = avgLight/50;
    if(avgLight>110){           //110 will be higher than ambient light and low enough to be reached if there are no beans
        return false;           //Return false if the sensor is uncovered by beans
    }
    return true;
}

void flipFilter(){//Flip the filter and place it back
    s1.write(0);
    delay(750);
    s1.write(30);
    delay(300);
    s1.write(0);
    delay(300);
    s1.write(30);
    delay(300);
    s1.write(0);
    delay(300);
    s1.write(30);
    delay(300);
    s1.write(0);
    delay(300);
    s1.write(30);
    delay(300);
    s1.write(0);
    delay(500);
    s1.write(180);
}

void setup() {
    pinMode(GRINDER_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(I_SENSE_PIN, INPUT);
    pinMode(srgPin, INPUT);
    s1.attach(servoPin);
    s1.write(180);
}

GrinderState grinderStatus = IDLE;

void loop() {
    now = millis();     // setting time
    lastRampTime = 0;   // setting ref point

    buttonState = digitalRead(BUTTON_PIN);      // allows for reading of the
    rawADC = analogRead(I_SENSE_PIN);           // current sening pin in a 
    voltageReading = rawADC*ADC_2_V;            // format that is friendly for
    currentReading = voltageReading * V_2_mA;   // testing and troubleshooting

    switch(grinderStatus) {

        case IDLE:          // do nothing
            if (buttonState == HIGH) {
                lastRampTime = now;
                grinderStatus = RAMP_UP;
            } else {
                currentSpeed = 0;
                analogWrite(GRINDER_PIN, currentSpeed);
            }
            break;
        
        case RAMP_UP:       // pwm up
            if (now - lastRampTime >= RAMP_INTERVAL) {
                lastRampTime = now;
                currentSpeed += 25;
                analogWrite(GRINDER_PIN, currentSpeed);
            } 

            if (currentSpeed >= MAX_CYCLE) {
                analogWrite(GRINDER_PIN, MAX_CYCLE);
                grinderStatus = STEADY_STATE;
            }
            break;

        case STEADY_STATE:  // hold constant speed
            grindTime = now;
            if (now - grindTime >= 2000) {
                grinderStatus = IDLE;
            }
            break;
    }
}