/*
  Code by: Pooya Daravi
  ..............
  TODO:
    - Add motor sleep mode capability
    - 

*/

#include <TimerOne.h>

#include <ThreadController.h>
#include <StaticThreadController.h>
#include <Thread.h>

// change these based on setup
#define X_DIR_POS true
#define Y_DIR_POS true
#define Z_DIR_POS true
#define PRESSED_DOWN 0
//Pin Layout
#define M1_DIR_PIN 22
#define M1_STEP_PIN 23
#define END_SWITCH_1 24
#define M2_DIR_PIN 26
#define M2_STEP_PIN 27
#define END_SWITCH_2 28
#define M3_DIR_PIN 8
#define M3_STEP_PIN 9
#define END_SWITCH_3 10

#define MAGNET_PIN 11
#define VIBRATION_PIN 12
#define HEATER_PIN 13
#define FANS_PIN 14
#define UV_PIN 15

// readability
#define STOP 0

bool  x_step_forward = true;
int   x_step_count   = 0;
int   x_limit        = 0;
int   x_pos          = 0;
float x_speed        = 0.0;
bool  y_step_forward = true;
int   y_step_count   = 0;
int   y_limit        = 0;
int   y_pos          = 0;
float y_speed        = 0.0;
bool  z_step_forward = true;
int   z_step_count   = 0;
int   z_limit        = 0;
int   z_pos          = 0;
float z_speed        = 0.0;

bool m1StepHigh  = false;
bool m2StepHigh  = false;
bool m3StepHigh  = false;
bool m1StepperOn = false;
bool m2StepperOn = false;
bool m3StepperOn = false;

bool crane_on     = false;
bool magnet_on    = false;
bool vibration_on = false;
bool heater_on    = false;
bool fans_on      = false;
bool uv_on        = false;
int magnet_lvl    = 0;

int switchOneState   = 0;
int switchTwoState   = 0;
int switchThreeState = 0;
bool endSwitchOneTriggered   = false;
bool endSwitchTwoTriggered   = false;
bool endSwitchThreeTriggered = false;

String inputString    = "";    // a String to hold incoming data
bool   stringComplete = false; // whether the string is complete

Thread* th_a_m1_step = new Thread();
Thread* th_a_m2_step = new Thread();
Thread* th_a_m3_step = new Thread();
Thread* th_s_endSwitch = new Thread();

StaticThreadController<4> controller (th_a_m1_step, th_a_m2_step, th_a_m3_step, th_s_endSwitch);
//StaticThreadController<2> controller (th_a_m1_step, th_a_m2_step);

void m1_step_callback() {
  if (m1StepHigh) {
    digitalWrite(M1_STEP_PIN, LOW);
    m1StepHigh = false;
  }
  else {
    if (m1StepperOn) {
      digitalWrite(M1_STEP_PIN, HIGH);
      m1StepHigh = true;
      if (x_step_forward) {
        x_step_count++;
        if (x_step_count >= x_limit) {
          m1StepperOn = false;
        }
      } else {
        x_step_count--;
        if (x_step_count <= x_limit) {
          m1StepperOn = false;
        }
      }
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
      if (y_step_forward) {
        y_step_count++;
        if (y_step_count >= y_limit) {
          m2StepperOn = false;
        }
      } else {
        y_step_count--;
        if (y_step_count <= y_limit) {
          m2StepperOn = false;
        }
      }
    }
  }
}

void m3_step_callback() {
  if (m3StepHigh) {
    digitalWrite(M3_STEP_PIN, LOW);
    m3StepHigh = false;
  }
  else {
    if (m3StepperOn) {
      digitalWrite(M3_STEP_PIN, HIGH);
      m3StepHigh = true;
      if (z_step_forward) {
        z_step_count++;
        if (z_step_count >= z_limit) {
          m3StepperOn = false;
        }
      } else {
        z_step_count--;
        if (z_step_count <= z_limit) {
          m3StepperOn = false;
        }
      }
    }
  }
}

void endSwitch_callback() {
  // trigger led toggle on switch high
  switchOneState   = digitalRead(END_SWITCH_1);
  switchTwoState   = digitalRead(END_SWITCH_2);
  switchThreeState = digitalRead(END_SWITCH_3);

  if (switchOneState == PRESSED_DOWN) {
    m1StepperOn = false;
    x_step_count = 0;
  }

  if (switchTwoState == PRESSED_DOWN) {
    m2StepperOn = false;
    y_step_count = 0;
  }

  if (switchThreeState == PRESSED_DOWN) {
    m3StepperOn = false;
    z_step_count = 0;
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
  
  /* Stepper motors */
  // X
  pinMode     ( M1_DIR_PIN,  OUTPUT );
  pinMode     ( M1_STEP_PIN, OUTPUT );
  digitalWrite( M1_DIR_PIN,  LOW    );
  digitalWrite( M1_STEP_PIN, LOW    );
  // Y
  pinMode     ( M2_DIR_PIN,  OUTPUT );
  pinMode     ( M2_STEP_PIN, OUTPUT );
  digitalWrite( M2_DIR_PIN,  LOW    );
  digitalWrite( M2_STEP_PIN, LOW    );
  // Z
  pinMode     ( M3_DIR_PIN,  OUTPUT );
  pinMode     ( M3_STEP_PIN, OUTPUT );
  digitalWrite( M3_DIR_PIN,  LOW    );
  digitalWrite( M3_STEP_PIN, LOW    );

  // Endstop(/limit) switch
  pinMode(END_SWITCH_1, INPUT);
  pinMode(END_SWITCH_2, INPUT);
  pinMode(END_SWITCH_3, INPUT);

  /* Threads */
  // X
  th_a_m1_step->setInterval(10000); // microseconds
  th_a_m1_step->onRun(m1_step_callback);
  // Y
  th_a_m2_step->setInterval(10000);
  th_a_m2_step->onRun(m2_step_callback);
  // Z
  th_a_m3_step->setInterval(10000);
  th_a_m3_step->onRun(m3_step_callback);
  // Endstops
  th_s_endSwitch->setInterval(10000);
  th_s_endSwitch->onRun(endSwitch_callback);

  // Timer setup
  Timer1.initialize(200);
  Timer1.attachInterrupt(timerCallback);
  Timer1.start();

}

void loop() {
  if (stringComplete) {
    // sample input: 0, 10303030505050000000

      


    // Serial.println(inputString));

    // actuation signals
    crane_on     = inputString.substring(0,  1).toInt();
    x_speed      = inputString.substring(1,  3).toInt();
    y_speed      = inputString.substring(3,  5).toInt();
    z_speed      = inputString.substring(5,  7).toInt();
    x_pos        = inputString.substring(7,  9).toInt();
    y_pos        = inputString.substring(9, 11).toInt();
    z_pos        = inputString.substring(11,13).toInt();
    magnet_on    = inputString.substring(13,14).toInt();
    magnet_lvl   = inputString.substring(14,16).toInt();
    vibration_on = inputString.substring(16,17).toInt();
    heater_on    = inputString.substring(17,18).toInt();
    fans_on      = inputString.substring(18,19).toInt();
    uv_on        = inputString.substring(19,20).toInt();

    if (magnet_on) {
      digitalWrite(MAGNET_PIN, HIGH);
    } else {
      digitalWrite(MAGNET_PIN, LOW);
    }

    if (vibration_on) {
      digitalWrite(VIBRATION_PIN, HIGH);
    } else {
      digitalWrite(VIBRATION_PIN, LOW);
    }

    if (heater_on) {
      digitalWrite(HEATER_PIN, HIGH);
    } else {
      digitalWrite(HEATER_PIN, LOW);
    }

    if (fans_on) {
      digitalWrite(FANS_PIN, HIGH);
    } else {
      digitalWrite(FANS_PIN, LOW);
    }

    if (uv_on) {
      digitalWrite(UV_PIN, HIGH);
    } else {
      digitalWrite(UV_PIN, LOW);
    }
    
    // set motor 1 speed
    if (!crane_on) {
      m1StepperOn = false;
    } 
    else {
      // X Control
      if (x_speed == STOP) {
        m1StepperOn = false;
      }
      else {
        // Set position limit (limit number of steps)
        x_limit = 100 * x_pos;
        // Set motor direction
        if (x_limit < x_step_count) {
          x_step_forward = !X_DIR_POS;
          digitalWrite(M1_DIR_PIN, LOW);
        }
        else {
          x_step_forward = X_DIR_POS;
          digitalWrite(M1_DIR_PIN, HIGH);
        }
        // Set speed
        th_a_m1_step->setInterval(4000.0/abs(x_speed)); // could go down to 3750
        m1StepperOn = true;
      }
      // Y Control
      if (y_speed == STOP) {
        m2StepperOn = false;
      }
      else {
        // Set position limit (limit number of steps)
        y_limit = 100 * y_pos;
        // Set motor direction
        if (y_limit < y_step_count) {
          y_step_forward = !Y_DIR_POS;
          digitalWrite(M2_DIR_PIN, LOW);
        }
        else {
          y_step_forward = Y_DIR_POS;
          digitalWrite(M2_DIR_PIN, HIGH);
        }
        // Set speed
        th_a_m2_step->setInterval(4000.0/abs(y_speed));
        m2StepperOn = true;
      }
      // Z Control
      if (z_speed == STOP) {
        m3StepperOn = false;
      }
      else {
        // Set position limit (limit number of steps)
        z_limit = 100 * z_pos;
        // Set motor direction
        if (z_limit < z_step_count) {
          z_step_forward = !Z_DIR_POS;
          digitalWrite(M3_DIR_PIN, LOW);
        }
        else {
          z_step_forward = Z_DIR_POS;
          digitalWrite(M3_DIR_PIN, HIGH);
        }
        // Set speed
        th_a_m3_step->setInterval(4000.0/abs(z_speed));
        m3StepperOn = true;
      }
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

