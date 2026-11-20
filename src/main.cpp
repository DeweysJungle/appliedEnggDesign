#include <Arduino.h>
#include <Servo.h>

#define storage A2
#define servo 5
#define button A5
#define pwm 11
#define redled 6
#define blueled 9
#define greenled 10
#define dir 12
#define led 7
#define encA 2
#define encB 3

// pwm vars
const int minCycle = 32;        // 1/8 max power
const int maxCycle = 255;       // 7/8 max power
const int rampInterval = 100;   // 100 ms
int currentSpeed = 0;

// state vars
bool buttonState;

// time variables
unsigned long now;
unsigned long lastRampTime;

// Encoder variables
volatile int encoderCount = 0;
volatile bool encoderMoved = false;
int grindtime;

void readEncoder() {
    if (digitalRead(encA) == digitalRead(encB)) {
        encoderCount++;
    } else {
        encoderCount--;
    }
    if(encoderCount>19){
        encoderCount = 0;
    }else if(encoderCount<0){
        encoderCount = 19;
    }

    encoderMoved = true;
}

void getGrindTime(){
    if(encoderCount<6){ //small
        grindtime = 20;
    }else if(encoderCount<12){ //medium
        grindtime = 30;
    }else{ //large
        grindtime = 40;
    }
}

enum GrinderState {
        IDLE,
        RAMP_UP,
        STEADY_STATE,
    };

GrinderState grinderStatus = IDLE;
Servo s1;

bool checkStorage(){
    int avgLight = 0;
    for(int i = 0;i < 50;i++){      // Take the avg of 50 readings
        avgLight += analogRead(storage);
    }
    avgLight = avgLight/50;
    if(avgLight>110){           
        return false;
    }
    return true;
}

void flipFilter() {
    s1.write(0);
    delay(750);
    for (int i = 1; i <= 3; i++) {
        s1.write(40);
        delay(300);
        s1.write(0);
        delay(300);
    }
    delay(500);
    s1.write(180);
}

void serialTimer(int t, int t_max, String verb) {
    Serial.print("\rTime until ");
    Serial.print(verb);
    Serial.print(" ends:    ");
    Serial.print(t_max - t);
    Serial.print(" sec");
}

void setup() {
    Serial.begin(9600);

    s1.attach(servo);
    s1.write(180);

    pinMode(storage, INPUT);

    pinMode(pwm, OUTPUT);
    pinMode(dir, OUTPUT);
    pinMode(button, INPUT);

    pinMode(led, OUTPUT);
    lastRampTime = 0;       // setting ref point

    pinMode(encA, INPUT_PULLUP);
    pinMode(encB, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(encA), readEncoder, FALLING);
}

void loop() {
    now = millis();         // setting time
    buttonState = digitalRead(button);
    digitalWrite(led, HIGH);
    getGrindTime();         //Read the button
    switch(grinderStatus) {

        case IDLE:          // do nothing
            if (buttonState == HIGH) {
                Serial.print("\nButton pressed!\n");
                lastRampTime = now;
                grinderStatus = RAMP_UP;
            } else {
                currentSpeed = 0;
                analogWrite(pwm, currentSpeed);
            }

            break;
        
        case RAMP_UP:       // pwm up
            if (now - lastRampTime >= rampInterval) {
                lastRampTime = now;
                currentSpeed += 25;
                analogWrite(pwm, currentSpeed);
            } 

            if (currentSpeed >= maxCycle) {
                grinderStatus = STEADY_STATE;
            }
            
            break;

        case STEADY_STATE:  // hold constant speed
            Serial.print("Grinding...\n");
            int t = 0;
            serialTimer(t, 20, "grinding");
            delay(500);
            for (int i = 0; i <= 4; i++) {
                analogWrite(pwm, 0);
                delay(500);             // 0.3 sec
                digitalWrite(dir, 1);
                analogWrite(pwm, maxCycle);
                delay(500);             // 0.3 sec
                t += 1;
                serialTimer(t, 20, "grinding");
                analogWrite(pwm, 0);
                delay(500);             // 0.3 sec
                digitalWrite(dir, 0);
                analogWrite(pwm, maxCycle);
                t += 1;
                serialTimer(t, 20, "grinding");
                delay(1000);            // 1.0 sec
                t += 1;
                serialTimer(t, 20, "grinding");
                delay(1000);            // 1.0 sec
                t += 1;
                serialTimer(t, 20, "grinding");
                delay(1000);            // 1.0 sec
            }
            analogWrite(pwm, 0);
            grinderStatus = IDLE;
            Serial.print("\n");
            Serial.print("Brewing...\n");
            for (int t = 0; t <=30; t++) {
                serialTimer(t, 30, "flip");
                delay(1000);
            }
            flipFilter();
            Serial.print("\nDone! Thank you!");
            break;
    }
}