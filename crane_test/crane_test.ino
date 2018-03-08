#include <TimerOne.h>

#include <ThreadController.h>
#include <StaticThreadController.h>
#include <Thread.h>




//Pin Layout
#define M1_DIR_PIN 6
#define M1_STEP_PIN 7
#define END_SWITCH_1 8
#define M2_DIR_PIN 2
#define M2_STEP_PIN 3
#define END_SWITCH_2 9

#define PRESSED_DOWN 0
#define STOP 0
 
bool m1StepHigh = false;
bool m2StepHigh = false;

bool endSwitchOneTriggered = false;
bool endSwitchTwoTriggered = false;

bool m1StepperOn = false;
bool m2StepperOn = false;

int switchOneState = 0;
int switchTwoState = 0;

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
float x_pos = 0.0;
float y_pos = 0.0;

Thread* th_a_m1_step = new Thread();
Thread* th_a_m2_step = new Thread();
Thread* th_s_endSwitch = new Thread();

//StaticThreadController<3> controller (th_a_m1_step, th_a_m2_step, th_s_endSwitch);
StaticThreadController<2> controller (th_a_m1_step, th_a_m2_step);

void m1_step_callback() {
  if (m1StepHigh) {
    digitalWrite(M1_STEP_PIN, LOW);
    m1StepHigh = false;
  }
  else {
    if (m1StepperOn) {
      digitalWrite(M1_STEP_PIN, HIGH);
      m1StepHigh = true;
    }
  }
}

void m2_step_callback() {
  if (m2StepHigh) {
    digitalWrite(M2_STEP_PIN, LOW);
    m2StepHigh = false;
  }
  else {
    if (m2StepperOn) {
      digitalWrite(M2_STEP_PIN, HIGH);
      m2StepHigh = true;
    }
  }
}

void endSwitch_callback() {
  // trigger led toggle on switch high
  switchOneState = digitalRead(END_SWITCH_1);
  switchTwoState = digitalRead(END_SWITCH_2);

  if (switchOneState == PRESSED_DOWN) {
    m1StepperOn = false;
  }

  if (switchTwoState == PRESSED_DOWN) {
    m2StepperOn = false;
  }

}

// This is the callback for the Timer
void timerCallback(){
  controller.run();
}

void setup() {
  // Serial
  Serial.begin(9600);
  inputString.reserve(200);
  
  // Stepper motors
  pinMode(M1_DIR_PIN, OUTPUT);
  pinMode(M1_STEP_PIN, OUTPUT);
  digitalWrite(M1_DIR_PIN, LOW);
  digitalWrite(M1_STEP_PIN, LOW);

  pinMode(M2_DIR_PIN, OUTPUT);
  pinMode(M2_STEP_PIN, OUTPUT);
  digitalWrite(M2_DIR_PIN, LOW);
  digitalWrite(M2_STEP_PIN, LOW);

  // Endstop(/limit) switch
  pinMode(END_SWITCH_1, INPUT);
  pinMode(END_SWITCH_2, INPUT);

  // Threads
  th_a_m1_step->setInterval(10000); // microseconds
  th_a_m1_step->onRun(m1_step_callback);
  th_a_m2_step->setInterval(10000); // microseconds
  th_a_m2_step->onRun(m2_step_callback);
  th_s_endSwitch->setInterval(10000);
  th_s_endSwitch->onRun(endSwitch_callback);

  // Timer setup
  Timer1.initialize(200);
  Timer1.attachInterrupt(timerCallback);
  Timer1.start();

}

void loop() {
  if (stringComplete) {

    x_pos = inputString.substring(0,2).toFloat();
//    Serial.println(abs(x_pos));
    y_pos = inputString.substring(2,4).toFloat();
//    Serial.println(abs(y_pos));
    
    // set motor 1 speed
    if (x_pos == STOP) {
      m1StepperOn = false;
    }
    else {
      if (x_pos < 0) {
        digitalWrite(M1_DIR_PIN, LOW);
      }
      else {
        digitalWrite(M1_DIR_PIN, HIGH);
      }
      th_a_m1_step->setInterval(4000.0/abs(x_pos)); // could go down to 3750
      m1StepperOn = true;
    }

    // set motor 2 speed
    if (y_pos == STOP) {
      m2StepperOn = false;
    }
    else {
      if (y_pos < 0) {
        digitalWrite(M2_DIR_PIN, LOW);
      }
      else {
        digitalWrite(M2_DIR_PIN, HIGH);
      }
      
      th_a_m2_step->setInterval(4000.0/abs(y_pos));
      m2StepperOn = true;
    }
    
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
}


void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

