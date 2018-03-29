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
#define X_POSITIVE 1
#define Y_POSITIVE 1
#define Z_POSITIVE 1

#define W_FORK      600
#define W_SPOON     400
#define W_KNIFE     800
#define W_TOLERANCE 0.5
#define W_SCALE     1.0

// Number of values to average for weight sensor filtering (1: no filtering, max: 10000)
#define WEIGHT_BUFFER_SIZE 100

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
#define LOAD_SENSOR_PIN    A0
#define MAGNET_PWM_PIN     2
#define POLISHER_PIN       53
#define MAGNET_POWER_PIN   52
#define HEATER_PIN         51
#define FANS_PIN           50
#define UV_PIN             49

/* readability */
#define STOP         0
#define PRESSED_DOWN 0
#define POSITIVE     1
#define NEGATIVE     0

// crane modes
#define OFF          0
#define RESET        1
#define MANUAL       2
#define AUTO         3



bool  x_direction = true;
int   x_position  = 0;
int   x_limit     = 0;
int   x_goal      = 0;
float x_speed     = 0.0;
bool  y_direction = true;
int   y_position  = 0;
int   y_limit     = 0;
int   y_goal      = 0;
float y_speed     = 0.0;
bool  z_direction = true;
int   z_position  = 0;
int   z_limit     = 1000;
int   z_goal      = 0;
float z_speed     = 0.0;
float weight      = 0.0;

// weigh filter variables
// int weights[FILTER_COUNT];
// memset(weights, 0, sizeof(weights));
// long w_running_total = 0;

enum LoadType { 
  NoLoad, 
  Fork, 
  Spoon, 
  Knive,
  Unknown
};
LoadType load = NoLoad;

bool mx_step_high  = false;
bool my_step_high  = false;
bool mz_step_high  = false;
bool mx_stepper_on = false;
bool my_stepper_on = false;
bool mz_stepper_on = false;

int  crane_mode   = false;
bool magnet_on    = false;
int  magnet_lvl   = 0;
bool vibration_on = false;
bool heater_on    = false;
bool fans_on      = false;
bool uv_on        = false;

int  switch_x_start = 0;
int  switch_x_end   = 0;
int  switch_y_start = 0;
int  switch_y_end   = 0;
int  switch_z_start = 0;
int  switch_z_end   = 0;

String commandString    = "";    // a String to hold incoming data
bool   stringComplete = false; // whether the string is complete

Thread* th_a_mx_step    = new Thread();
Thread* th_a_my_step    = new Thread();
Thread* th_a_mz_step    = new Thread();
Thread* th_s_end_switch = new Thread();
Thread* th_s_detect     = new Thread();

// StaticThreadController<5> controller (th_s_detect, th_a_mx_step, th_a_my_step, th_a_mz_step, th_s_end_switch);
StaticThreadController<4> controller (th_s_detect, th_a_mx_step, th_a_my_step, th_a_mz_step);
// StaticThreadController<1> controller (th_a_mx_step);

void detect_callback() {

  // if (count >= FILTER_COUNT) {
  //   w_average = w_total * 1.0 / FILTER_COUNT; // cast to float
  // }




  weight = analogRead(LOAD_SENSOR_PIN) * W_SCALE;
  // Serial.print("Load value: ");
  // Serial.println(weight);
  if (weight < 10 * W_TOLERANCE) {
    load = NoLoad;
  } else if (weight > (W_FORK - W_TOLERANCE) && weight < (W_FORK + W_TOLERANCE)) {
    load = Fork;
  } else if (weight > (W_SPOON - W_TOLERANCE) && weight < (W_SPOON + W_TOLERANCE)) {
    load = Spoon;
  } else if (weight > (W_KNIFE - W_TOLERANCE) && weight < (W_KNIFE + W_TOLERANCE)) {
    load = Knive;
  } else {
    load = Unknown;
  }

  switch (load) {
      case NoLoad:
        Serial.println("No load detected.");
        break;
      case Fork:
        Serial.println("Detected load: Fork");
        break;
      case Spoon:
        Serial.println("Detected load: Spoon");
        break;
      case Knive:
        Serial.println("Detected load: Knive");
        break;
      case Unknown:
        Serial.println("Detected load: Unknown");
        break;
      default:
        break;
        // do something
  }
}

void mx_step_callback() {
  if (mx_step_high) {
    digitalWrite(MX_STEP_PIN, LOW);
    // Serial.println("x low");
    mx_step_high = false;
  }
  else {
    if (mx_stepper_on) {
      digitalWrite(MX_STEP_PIN, HIGH);
      // Serial.println("x high");
      mx_step_high = true;
      if (x_direction == POSITIVE) {
        x_position++;
        // Serial.print("x_position ");
        // Serial.println(x_position);
        if (x_position >= x_goal) {
          mx_stepper_on = false;
        }
      } else {
        x_position--;
        // Serial.print("x_position ");
        // Serial.println(x_position);
        if (x_position <= x_goal) {
          mx_stepper_on = false;
        }
      }
    }
  }
}

void my_step_callback() {
  if (my_step_high) {
    digitalWrite(MY_STEP_PIN, LOW);
    my_step_high = false;
  }
  else {
    if (my_stepper_on) {
      digitalWrite(MY_STEP_PIN, HIGH);
      my_step_high = true;
      if (y_direction == POSITIVE) {
        y_position++;
        // Serial.print("y_position");
        // Serial.println(y_position);
        if (y_position >= y_goal) {
          my_stepper_on = false;
        }
      } else {
        y_position--;
        if (y_position <= y_goal) {
          my_stepper_on = false;
        }
      }
    }
  }
}

void mz_step_callback() {
  if (mz_step_high) {
    digitalWrite(MZ_STEP_PIN, LOW);
    mz_step_high = false;
  }
  else {
    if (mz_stepper_on) {
      digitalWrite(MZ_STEP_PIN, HIGH);
      mz_step_high = true;
      if (z_direction == POSITIVE) {
        z_position++;
        // Serial.print("z_position");
        // Serial.println(z_position);
        if (z_position >= z_goal) {
          mz_stepper_on = false;
        }
      } else {
        z_position--;
        if (z_position <= z_goal) {
          mz_stepper_on = false;
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
    Serial.println("x_start pressed");
    if (x_direction == NEGATIVE) {
      mx_stepper_on = false;
      x_position = 0;
    }
  }

  if (switch_x_end == PRESSED_DOWN) {
    Serial.println("x_end pressed");
    if (x_direction == POSITIVE) {
      mx_stepper_on = false;
      x_limit       = x_position;
    }
  }

  if (switch_y_start == PRESSED_DOWN) {
    Serial.println("y_start pressed");
    if (y_direction == NEGATIVE) {
      my_stepper_on = false;
      y_position = 0;
    }
  }

  if (switch_y_end == PRESSED_DOWN) {
    Serial.println("y_end pressed");
    if (y_direction == POSITIVE) {
      my_stepper_on = false;
      y_limit       = y_position;
    }
  }

  if (switch_z_start == PRESSED_DOWN) {
    Serial.println("z_start pressed");
    if (z_direction == NEGATIVE) {
      mz_stepper_on = false;
      z_position = 0;
    }
  }

  if (switch_z_end == PRESSED_DOWN) {
    Serial.println("z_end pressed");
    if (z_direction == POSITIVE) {
      mz_stepper_on = false;
      z_limit       = z_position;
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
  commandString.reserve(200);
  
  /* Pins */
  // X
  pinMode     ( MX_DIR_PIN,         OUTPUT );
  digitalWrite( MX_DIR_PIN,         LOW    );
  pinMode     ( MX_STEP_PIN,        OUTPUT );
  digitalWrite( MX_STEP_PIN,        LOW    );
  pinMode     ( END_SWITCH_X_START, INPUT  );
  pinMode     ( END_SWITCH_X_END,   INPUT  );
  // // Y
  pinMode     ( MY_DIR_PIN,         OUTPUT );
  digitalWrite( MY_DIR_PIN,         LOW    );
  pinMode     ( MY_STEP_PIN,        OUTPUT );
  digitalWrite( MY_STEP_PIN,        LOW    );
  pinMode     ( END_SWITCH_Y_START, INPUT  );
  pinMode     ( END_SWITCH_Y_END,   INPUT  );
  // // Z
  pinMode     ( MZ_DIR_PIN,         OUTPUT );
  digitalWrite( MZ_DIR_PIN,         LOW    );
  pinMode     ( MZ_STEP_PIN,        OUTPUT );
  digitalWrite( MZ_STEP_PIN,        LOW    );
  pinMode     ( END_SWITCH_Z_START, INPUT  );
  pinMode     ( END_SWITCH_Z_END,   INPUT  );
  // Other
  pinMode     ( LOAD_SENSOR_PIN,    INPUT  );
  pinMode     ( MAGNET_PWM_PIN,     OUTPUT );
  analogWrite ( MAGNET_PWM_PIN,     0      );
  pinMode     ( MAGNET_POWER_PIN,   OUTPUT );
  digitalWrite( MAGNET_POWER_PIN,   LOW    );
  pinMode     ( POLISHER_PIN,       OUTPUT );
  digitalWrite( POLISHER_PIN,       LOW    );
  pinMode     ( HEATER_PIN,         OUTPUT );
  digitalWrite( HEATER_PIN,         LOW    );
  pinMode     ( FANS_PIN,           OUTPUT );
  digitalWrite( FANS_PIN,           LOW    );
  pinMode     ( UV_PIN,             OUTPUT );
  digitalWrite( UV_PIN,             LOW    );

  /* Threads */
  // X
  th_a_mx_step->setInterval(10000); // microseconds
  th_a_mx_step->onRun(mx_step_callback);
  // Y
  th_a_my_step->setInterval(10000);
  th_a_my_step->onRun(my_step_callback);
  // Z
  th_a_mz_step->setInterval(10000);
  th_a_mz_step->onRun(mz_step_callback);
  // Endstops
  th_s_end_switch->setInterval(10000);
  th_s_end_switch->onRun(endSwitch_callback);
  // Weight Sensor
  th_s_detect->setInterval(4000000);
  th_s_detect->onRun(detect_callback);

  // Timer setup
  Timer1.initialize(200);
  Timer1.attachInterrupt(timerCallback);
  Timer1.start();
}


// void loop() {
//   if (stringComplete) {
//     // sample inputs: 
//     // off: 0
//     // reset: 1
//     // manual, speeds:3, positions: 5, else: off --> 0, 201010102020200000000

//     // Debug
//     Serial.println(commandString);

//     // Decoding the input command into actuation signals
//     crane_mode   = commandString.substring(0,  1).toInt();
//     x_speed      = commandString.substring(1,  3).toInt();
//     y_speed      = commandString.substring(3,  5).toInt();
//     z_speed      = commandString.substring(5,  7).toInt();
//     x_goal       = commandString.substring(7,  9).toInt();
//     y_goal       = commandString.substring(9, 11).toInt();
//     z_goal       = commandString.substring(11,13).toInt();
//     magnet_on    = commandString.substring(13,14).toInt();
//     magnet_lvl   = commandString.substring(14,17).toInt();
//     vibration_on = commandString.substring(17,18).toInt();
//     heater_on    = commandString.substring(18,19).toInt();
//     fans_on      = commandString.substring(19,20).toInt();
//     uv_on        = commandString.substring(20,21).toInt();
    
//     Serial.print("Crane mode: ");
//     Serial.println(crane_mode);
//     Serial.print("X speed: ");
//     Serial.println(x_speed);
//     Serial.print("Y speed: ");
//     Serial.println(y_speed);
//     Serial.print("Z speed: ");
//     Serial.println(z_speed);
//     Serial.print("X goal: ");
//     Serial.println(x_goal);
//     Serial.print("Y goal: ");
//     Serial.println(y_goal);
//     Serial.print("Z goal: ");
//     Serial.println(z_goal);
//     Serial.print("magnet_on: ");
//     Serial.println(magnet_on);
//     Serial.print("magnet_lvl: ");
//     Serial.println(magnet_lvl);
//     Serial.print("vibration_on: ");
//     Serial.println(vibration_on);
//     Serial.print("heater_on: ");
//     Serial.println(heater_on);
//     Serial.print("fans_on: ");
//     Serial.println(fans_on);
//     Serial.print("uv_on: ");
//     Serial.println(uv_on);
    


//     switch (crane_mode) {


//         case OFF:
//           /*
//             Off mode: turn all components off immediately
//           */

//           mx_stepper_on = false;
//           my_stepper_on = false;
//           mz_stepper_on = false;
//           analogWrite (MAGNET_PWM_PIN,   0  );
//           digitalWrite(MAGNET_POWER_PIN, LOW);
//           digitalWrite(POLISHER_PIN,     LOW);
//           digitalWrite(HEATER_PIN,       LOW);
//           digitalWrite(FANS_PIN,         LOW);
//           digitalWrite(UV_PIN,           LOW);

//           break;


//         case RESET:
//           /*
//             Reset mode:
//               - go to position (0, 0) and reset the counts to zero
//               - go to position (end, end) and set limits
//               - turn all else off
//           */

//           // Debug
//           Serial.println("In reset mode...");

//           analogWrite (MAGNET_PWM_PIN,   0  );
//           digitalWrite(MAGNET_POWER_PIN, LOW);
//           digitalWrite(POLISHER_PIN,     LOW);
//           digitalWrite(HEATER_PIN,       LOW);
//           digitalWrite(FANS_PIN,         LOW);
//           digitalWrite(UV_PIN,           LOW);

//           // set limit and direction
//           x_goal = -10000;
//           y_goal = -10000;
//           z_goal = -10000;
//           x_direction = !X_POSITIVE;
//           digitalWrite(MX_DIR_PIN, !X_POSITIVE);
//           y_direction = !Y_POSITIVE;
//           digitalWrite(MY_DIR_PIN, !Y_POSITIVE);
//           z_direction = !Z_POSITIVE;
//           digitalWrite(MZ_DIR_PIN, !Z_POSITIVE);
//           // Set speed
//           th_a_mx_step->setInterval(500.0);
//           th_a_my_step->setInterval(500.0);
//           th_a_mz_step->setInterval(500.0);
//           mx_stepper_on = true;
//           my_stepper_on = true;
//           mz_stepper_on = true;
//           while (mx_stepper_on || my_stepper_on || mz_stepper_on) {
//             // Debug
//             Serial.print("x_position");
//             Serial.println(x_position);
//           };

//           // set limit and direction
//           x_goal = 10000;
//           y_goal = 10000;
//           z_goal = 10000;
//           x_direction = X_POSITIVE;
//           digitalWrite(MX_DIR_PIN, X_POSITIVE);
//           y_direction = Y_POSITIVE;
//           digitalWrite(MY_DIR_PIN, Y_POSITIVE);
//           // z_direction = Z_POSITIVE;
//           // digitalWrite(MZ_DIR_PIN, Z_POSITIVE);
//           // Set speed
//           th_a_mx_step->setInterval(500.0);
//           th_a_my_step->setInterval(500.0);
//           // th_a_mz_step->setInterval(500.0);
//           mx_stepper_on = true;
//           my_stepper_on = true;
//           // mz_stepper_on = true;
//           while (mx_stepper_on || my_stepper_on || mz_stepper_on) {
//             // Debug
//             Serial.print("x_position");
//             Serial.println(x_position);
//           };

//           Serial.print("x_limit: ");
//           Serial.println(x_limit);
//           Serial.print("y_limit: ");
//           Serial.println(y_limit);
//           Serial.print("z_limit: ");
//           Serial.println(z_limit);
        

//         case MANUAL:
//           /*
//             Manual mode: set all settings according to input 
//             recieved from serial.
//           */

//           // Debug
//           Serial.println("in manual mode...");

//           analogWrite (MAGNET_PWM_PIN,   magnet_lvl  );
//           digitalWrite(MAGNET_POWER_PIN, magnet_on   );
//           digitalWrite(POLISHER_PIN,     vibration_on);
//           digitalWrite(HEATER_PIN,       heater_on   );
//           digitalWrite(FANS_PIN,         fans_on     );
//           digitalWrite(UV_PIN,           uv_on       );

//           // X Control
//           if (x_speed == STOP) {
//             mx_stepper_on = false;
//           }
//           else {
//             // Set position limit (limit number of steps)
//             x_goal = 100 * x_goal;
//             // Set motor direction
//             if (x_goal < x_position) {
//               x_direction = !X_POSITIVE;
//               digitalWrite(MX_DIR_PIN, LOW);
//               Serial.println("changing dir to low");
//             }
//             else {
//               x_direction = X_POSITIVE;
//               digitalWrite(MX_DIR_PIN, HIGH);
//               Serial.println("changing dir to high");
//             }
//             // Set speed
//             th_a_mx_step->setInterval(4000.0/abs(x_speed)); // could go down to 3750
//             mx_stepper_on = true;
//           }
//           // Y Control
//           if (y_speed == STOP) {
//             my_stepper_on = false;
//           }
//           else {
//             // Set position limit (limit number of steps)
//             y_goal = 100 * y_goal;
//             // Set motor direction
//             if (y_goal < y_position) {
//               y_direction = !Y_POSITIVE;
//               digitalWrite(MY_DIR_PIN, LOW);
//             }
//             else {
//               y_direction = Y_POSITIVE;
//               digitalWrite(MY_DIR_PIN, HIGH);
//             }
//             // Set speed
//             th_a_my_step->setInterval(4000.0/abs(y_speed));
//             my_stepper_on = true;
//           }
//           // Z Control
//           if (z_speed == STOP) {
//             mz_stepper_on = false;
//           }
//           else {
//             // Set position limit (limit number of steps)
//             z_goal = 100 * z_goal;
//             // Set motor direction
//             if (z_goal < z_position) {
//               z_direction = !Z_POSITIVE;
//               digitalWrite(MZ_DIR_PIN, LOW);
//             }
//             else {
//               z_direction = Z_POSITIVE;
//               digitalWrite(MZ_DIR_PIN, HIGH);
//             }
//             // Set speed
//             th_a_mz_step->setInterval(4000.0/abs(z_speed));
//             mz_stepper_on = true;
//           }

//           break;


//         case AUTO:
//           /*
//             Auto mode: final design
//             TODO - implement auto based on designed FSM
//           */
//           break;


//         default:
//           break;
//           // do something
//     }
    
//     // clear the string:
//     commandString = "";
//     stringComplete = false;
//   }
// }

void loop() {
  if (stringComplete) {
    // sample inputs: 
    // off: 0
    // reset: 1
    // manual, speeds:3, positions: 5, else: off --> 0, 201010102020200000000

    // Debug
    Serial.println(commandString);

    // Decoding the input command into actuation signals
    crane_mode   = commandString.substring(0,  1).toInt();
    x_speed      = commandString.substring(1,  3).toInt();
    y_speed      = commandString.substring(3,  5).toInt();
    z_speed      = commandString.substring(5,  7).toInt();
    x_goal       = commandString.substring(7,  9).toInt();
    y_goal       = commandString.substring(9, 11).toInt();
    z_goal       = commandString.substring(11,13).toInt();
    magnet_on    = commandString.substring(13,14).toInt();
    magnet_lvl   = commandString.substring(14,17).toInt();
    vibration_on = commandString.substring(17,18).toInt();
    heater_on    = commandString.substring(18,19).toInt();
    fans_on      = commandString.substring(19,20).toInt();
    uv_on        = commandString.substring(20,21).toInt();
    
    Serial.print("Crane mode: ");
    Serial.println(crane_mode);
    Serial.print("X speed: ");
    Serial.println(x_speed);
    Serial.print("Y speed: ");
    Serial.println(y_speed);
    Serial.print("Z speed: ");
    Serial.println(z_speed);
    Serial.print("X goal: ");
    Serial.println(x_goal);
    Serial.print("Y goal: ");
    Serial.println(y_goal);
    Serial.print("Z goal: ");
    Serial.println(z_goal);
    Serial.print("magnet_on: ");
    Serial.println(magnet_on);
    Serial.print("magnet_lvl: ");
    Serial.println(magnet_lvl);
    Serial.print("vibration_on: ");
    Serial.println(vibration_on);
    Serial.print("heater_on: ");
    Serial.println(heater_on);
    Serial.print("fans_on: ");
    Serial.println(fans_on);
    Serial.print("uv_on: ");
    Serial.println(uv_on);
    


    switch (crane_mode) {


        case OFF:
          /*
            Off mode: turn all components off immediately
          */

          mx_stepper_on = false;
          my_stepper_on = false;
          mz_stepper_on = false;
          analogWrite (MAGNET_PWM_PIN,   0  );
          digitalWrite(MAGNET_POWER_PIN, LOW);
          digitalWrite(POLISHER_PIN,     LOW);
          digitalWrite(HEATER_PIN,       LOW);
          digitalWrite(FANS_PIN,         LOW);
          digitalWrite(UV_PIN,           LOW);

          break;


        case RESET:
          /*
            Reset mode:
              - go to position (0, 0) and reset the counts to zero
              - go to position (end, end) and set limits
              - turn all else off
          */

          // Debug
          Serial.println("In reset mode...");

          analogWrite (MAGNET_PWM_PIN,   0  );
          digitalWrite(MAGNET_POWER_PIN, LOW);
          digitalWrite(POLISHER_PIN,     LOW);
          digitalWrite(HEATER_PIN,       LOW);
          digitalWrite(FANS_PIN,         LOW);
          digitalWrite(UV_PIN,           LOW);

          // set limit and direction
          x_goal = -10000;
          y_goal = -10000;
          z_goal = -10000;
          x_direction = !X_POSITIVE;
          digitalWrite(MX_DIR_PIN, !X_POSITIVE);
          y_direction = !Y_POSITIVE;
          digitalWrite(MY_DIR_PIN, !Y_POSITIVE);
          z_direction = !Z_POSITIVE;
          digitalWrite(MZ_DIR_PIN, !Z_POSITIVE);
          // Set speed
          th_a_mx_step->setInterval(500.0);
          th_a_my_step->setInterval(500.0);
          th_a_mz_step->setInterval(500.0);
          mx_stepper_on = true;
          my_stepper_on = true;
          mz_stepper_on = true;
          while (mx_stepper_on || my_stepper_on || mz_stepper_on) {
            // Debug
            Serial.print("x_position");
            Serial.println(x_position);
          };

          // set limit and direction
          x_goal = 10000;
          y_goal = 10000;
          z_goal = 10000;
          x_direction = X_POSITIVE;
          digitalWrite(MX_DIR_PIN, X_POSITIVE);
          y_direction = Y_POSITIVE;
          digitalWrite(MY_DIR_PIN, Y_POSITIVE);
          // z_direction = Z_POSITIVE;
          // digitalWrite(MZ_DIR_PIN, Z_POSITIVE);
          // Set speed
          th_a_mx_step->setInterval(500.0);
          th_a_my_step->setInterval(500.0);
          // th_a_mz_step->setInterval(500.0);
          mx_stepper_on = true;
          my_stepper_on = true;
          // mz_stepper_on = true;
          while (mx_stepper_on || my_stepper_on || mz_stepper_on) {
            // Debug
            Serial.print("x_position");
            Serial.println(x_position);
          };

          Serial.print("x_limit: ");
          Serial.println(x_limit);
          Serial.print("y_limit: ");
          Serial.println(y_limit);
          Serial.print("z_limit: ");
          Serial.println(z_limit);
        

        case MANUAL:
          /*
            Manual mode: set all settings according to input 
            recieved from serial.
          */

          // Debug
          Serial.println("in manual mode...");

          analogWrite (MAGNET_PWM_PIN,   magnet_lvl  );
          digitalWrite(MAGNET_POWER_PIN, magnet_on   );
          digitalWrite(POLISHER_PIN,     vibration_on);
          digitalWrite(HEATER_PIN,       heater_on   );
          digitalWrite(FANS_PIN,         fans_on     );
          digitalWrite(UV_PIN,           uv_on       );

          // X Control
          if (x_speed == STOP) {
            mx_stepper_on = false;
          }
          else {
            // Set position limit (limit number of steps)
            x_goal = 100 * x_goal;
            // Set motor direction
            if (x_goal < x_position) {
              x_direction = !X_POSITIVE;
              digitalWrite(MX_DIR_PIN, LOW);
              Serial.println("changing dir to low");
            }
            else {
              x_direction = X_POSITIVE;
              digitalWrite(MX_DIR_PIN, HIGH);
              Serial.println("changing dir to high");
            }
            // Set speed
            th_a_mx_step->setInterval(4000.0/abs(x_speed)); // could go down to 3750
            mx_stepper_on = true;
          }
          // Y Control
          if (y_speed == STOP) {
            my_stepper_on = false;
          }
          else {
            // Set position limit (limit number of steps)
            y_goal = 100 * y_goal;
            // Set motor direction
            if (y_goal < y_position) {
              y_direction = !Y_POSITIVE;
              digitalWrite(MY_DIR_PIN, LOW);
            }
            else {
              y_direction = Y_POSITIVE;
              digitalWrite(MY_DIR_PIN, HIGH);
            }
            // Set speed
            th_a_my_step->setInterval(4000.0/abs(y_speed));
            my_stepper_on = true;
          }
          // Z Control
          if (z_speed == STOP) {
            mz_stepper_on = false;
          }
          else {
            // Set position limit (limit number of steps)
            z_goal = 100 * z_goal;
            // Set motor direction
            if (z_goal < z_position) {
              z_direction = !Z_POSITIVE;
              digitalWrite(MZ_DIR_PIN, LOW);
            }
            else {
              z_direction = Z_POSITIVE;
              digitalWrite(MZ_DIR_PIN, HIGH);
            }
            // Set speed
            th_a_mz_step->setInterval(4000.0/abs(z_speed));
            mz_stepper_on = true;
          }

          break;


        case AUTO:
          /*
            Auto mode: final design
            TODO - implement auto based on designed FSM
          */
          break;


        default:
          break;
          // do something
    }
    
    // clear the string:
    commandString = "";
    stringComplete = false;
  }
}

void serialEvent() {
  // Read command from serial port
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the commandString:
    commandString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

