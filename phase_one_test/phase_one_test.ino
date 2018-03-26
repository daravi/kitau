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

/* Pin Layout */
// Crane
#define MX_DIR_PIN         22
#define MX_STEP_PIN        23
#define END_SWITCH_X_START 24
#define END_SWITCH_X_END   25
#define MY_DIR_PIN         26
#define MY_STEP_PIN        27
#define END_SWITCH_Y_START 28
#define END_SWITCH_Y_END   29
#define MZ_DIR_PIN         30
#define MZ_STEP_PIN        31
#define END_SWITCH_Z_START 32
#define END_SWITCH_Z_END   33
// Other
#define MAGNET_PIN         10
#define VIBRATION_PIN      31
#define HEATER_PIN         32
#define FANS_PIN           33
#define UV_PIN             34

// readability
#define STOP         0
#define PRESSED_DOWN 0
#define POSITIVE     1
#define NEGATIVE     0

bool  x_direction  = true;
int   x_step_count = 0;
int   x_limit      = 0;
int   x_pos        = 0;
float x_speed      = 0.0;
bool  y_direction  = true;
int   y_step_count = 0;
int   y_limit      = 0;
int   y_pos        = 0;
float y_speed      = 0.0;
bool  z_direction  = true;
int   z_step_count = 0;
int   z_limit      = 0;
int   z_pos        = 0;
float z_speed      = 0.0;

bool m_x_step_high  = false;
bool m_y_step_high  = false;
bool m_z_step_high  = false;
bool m_x_stepper_on = false;
bool m_y_stepper_on = false;
bool m_z_stepper_on = false;

bool crane_on     = false;
bool magnet_on    = false;
bool vibration_on = false;
bool heater_on    = false;
bool fans_on      = false;
bool uv_on        = false;
int  magnet_lvl   = 0;

int  switch_x_start            = 0;
int  switch_x_end              = 0;
int  switch_y_start            = 0;
int  switch_y_end              = 0;
int  switch_z_start            = 0;
int  switch_z_end              = 0;

String inputString    = "";    // a String to hold incoming data
bool   stringComplete = false; // whether the string is complete

Thread* th_a_m_x_step   = new Thread();
Thread* th_a_m_y_step   = new Thread();
Thread* th_a_m_z_step   = new Thread();
Thread* th_s_end_switch = new Thread();

StaticThreadController<4> controller (th_a_m_x_step, th_a_m_y_step, th_a_m_z_step, th_s_end_switch);
//StaticThreadController<2> controller (th_a_m_x_step, th_a_m_y_step);

void mx_step_callback() {
  if (m_x_step_high) {
    digitalWrite(MX_STEP_PIN, LOW);
    m_x_step_high = false;
  }
  else {
    if (m_x_stepper_on) {
      digitalWrite(MX_STEP_PIN, HIGH);
      m_x_step_high = true;
      if (x_direction == POSITIVE) {
        x_step_count++;
        if (x_step_count >= x_limit) {
          m_x_stepper_on = false;
        }
      } else {
        x_step_count--;
        if (x_step_count <= x_limit) {
          m_x_stepper_on = false;
        }
      }
    }
  }
}

void my_step_callback() {
  if (m_y_step_high) {
    digitalWrite(MY_STEP_PIN, LOW);
    m_y_step_high = false;
  }
  else {
    if (m_y_stepper_on) {
      digitalWrite(MY_STEP_PIN, HIGH);
      m_y_step_high = true;
      if (y_direction == POSITIVE) {
        y_step_count++;
        if (y_step_count >= y_limit) {
          m_y_stepper_on = false;
        }
      } else {
        y_step_count--;
        if (y_step_count <= y_limit) {
          m_y_stepper_on = false;
        }
      }
    }
  }
}

void mz_step_callback() {
  if (m_z_step_high) {
    digitalWrite(MZ_STEP_PIN, LOW);
    m_z_step_high = false;
  }
  else {
    if (m_z_stepper_on) {
      digitalWrite(MZ_STEP_PIN, HIGH);
      m_z_step_high = true;
      if (z_direction == POSITIVE) {
        z_step_count++;
        if (z_step_count >= z_limit) {
          m_z_stepper_on = false;
        }
      } else {
        z_step_count--;
        if (z_step_count <= z_limit) {
          m_z_stepper_on = false;
        }
      }
    }
  }
}

void endSwitch_callback() {
  // trigger led toggle on switch high
  switch_x_start = digitalRead(END_SWITCH_X_START);
  switch_x_end   = digitalRead(END_SWITCH_X_END);
  switch_y_start = digitalRead(END_SWITCH_Y_START);
  switch_y_end   = digitalRead(END_SWITCH_Y_END);
  switch_z_start = digitalRead(END_SWITCH_Z_START);
  switch_z_end   = digitalRead(END_SWITCH_Z_END);

  if (switch_x_start == PRESSED_DOWN) {
    if (x_direction == NEGATIVE) {
      m_x_stepper_on = false;
      x_step_count = 0;
    }
  }

  if (switch_x_end == PRESSED_DOWN) {
    if (x_direction == POSITIVE) {
      m_x_stepper_on = false;
      x_max       = x_step_count;
    }
  }

  if (switch_y_start == PRESSED_DOWN) {
    if (y_direction == NEGATIVE) {
      m_y_stepper_on = false;
      y_step_count = 0;
    }
  }

  if (switch_y_end == PRESSED_DOWN) {
    if (y_direction == POSITIVE) {
      m_y_stepper_on = false;
      y_max       = y_step_count;
    }
  }

  if (switch_z_start == PRESSED_DOWN) {
    if (z_direction == NEGATIVE) {
      m_z_stepper_on = false;
      z_step_count = 0;
    }
  }

  if (switch_z_end == PRESSED_DOWN) {
    if (z_direction == POSITIVE) {
      m_z_stepper_on = false;
      z_max       = z_step_count;
    }
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
  pinMode     ( MX_DIR_PIN,  OUTPUT );
  pinMode     ( MX_STEP_PIN, OUTPUT );
  digitalWrite( MX_DIR_PIN,  LOW    );
  digitalWrite( MX_STEP_PIN, LOW    );
  // Y
  pinMode     ( MY_DIR_PIN,  OUTPUT );
  pinMode     ( MY_STEP_PIN, OUTPUT );
  digitalWrite( MY_DIR_PIN,  LOW    );
  digitalWrite( MY_STEP_PIN, LOW    );
  // Z
  pinMode     ( MZ_DIR_PIN,  OUTPUT );
  pinMode     ( MZ_STEP_PIN, OUTPUT );
  digitalWrite( MZ_DIR_PIN,  LOW    );
  digitalWrite( MZ_STEP_PIN, LOW    );

  // Endstop(/limit) switch
  pinMode(END_SWITCH_X_START, INPUT);
  pinMode(END_SWITCH_Y_START, INPUT);
  pinMode(END_SWITCH_Z_START, INPUT);

  /* Threads */
  // X
  th_a_m_x_step->setInterval(10000); // microseconds
  th_a_m_x_step->onRun(mx_step_callback);
  // Y
  th_a_m_y_step->setInterval(10000);
  th_a_m_y_step->onRun(my_step_callback);
  // Z
  th_a_m_z_step->setInterval(10000);
  th_a_m_z_step->onRun(mz_step_callback);
  // Endstops
  th_s_end_switch->setInterval(10000);
  th_s_end_switch->onRun(endSwitch_callback);

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
      m_x_stepper_on = false;
    } 
    else {
      // X Control
      if (x_speed == STOP) {
        m_x_stepper_on = false;
      }
      else {
        // Set position limit (limit number of steps)
        x_limit = 100 * x_pos;
        // Set motor direction
        if (x_limit < x_step_count) {
          x_direction = !X_DIR_POS;
          digitalWrite(MX_DIR_PIN, LOW);
        }
        else {
          x_direction = X_DIR_POS;
          digitalWrite(MX_DIR_PIN, HIGH);
        }
        // Set speed
        th_a_m_x_step->setInterval(4000.0/abs(x_speed)); // could go down to 3750
        m_x_stepper_on = true;
      }
      // Y Control
      if (y_speed == STOP) {
        m_y_stepper_on = false;
      }
      else {
        // Set position limit (limit number of steps)
        y_limit = 100 * y_pos;
        // Set motor direction
        if (y_limit < y_step_count) {
          y_direction = !Y_DIR_POS;
          digitalWrite(MY_DIR_PIN, LOW);
        }
        else {
          y_direction = Y_DIR_POS;
          digitalWrite(MY_DIR_PIN, HIGH);
        }
        // Set speed
        th_a_m_y_step->setInterval(4000.0/abs(y_speed));
        m_y_stepper_on = true;
      }
      // Z Control
      if (z_speed == STOP) {
        m_z_stepper_on = false;
      }
      else {
        // Set position limit (limit number of steps)
        z_limit = 100 * z_pos;
        // Set motor direction
        if (z_limit < z_step_count) {
          z_direction = !Z_DIR_POS;
          digitalWrite(MZ_DIR_PIN, LOW);
        }
        else {
          z_direction = Z_DIR_POS;
          digitalWrite(MZ_DIR_PIN, HIGH);
        }
        // Set speed
        th_a_m_z_step->setInterval(4000.0/abs(z_speed));
        m_z_stepper_on = true;
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

