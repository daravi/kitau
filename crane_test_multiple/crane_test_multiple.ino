#include <TimerOne.h>

#include <ThreadController.h>
#include <StaticThreadController.h>
#include <Thread.h>




#define STEP_PIN 10
#define DIR_PIN 6
#define END_SWITCH_1 11
#define END_SWITCH_2 12
 
//bool dirHigh;
bool stepHigh;
bool ledHigh;


bool endSwitchOneTriggered;
bool endSwitchTwoTriggered;
//bool shouldToggle = false;;
bool motorShouldStop = false;
bool stepperOn = false;
int switchOneState = 0;
int switchTwoState = 0;
int PRESSED_DOWN = 0;

String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
float x = 1.0;

Thread* th_a_step = new Thread();
Thread* th_s_serial = new Thread();
Thread* th_s_endSwitch = new Thread();

StaticThreadController<3> controller (th_a_step, th_s_serial, th_s_endSwitch);

void step_callback() {
  if (stepHigh) {
    digitalWrite(STEP_PIN, LOW);
    stepHigh = false;
  }
  else {
    if (stepperOn) {
      digitalWrite(STEP_PIN, HIGH);
      stepHigh = true;
    }
  }
//  digitalWrite(STEP_PIN, HIGH);
//  delayMicroseconds(520);
//  digitalWrite(STEP_PIN, LOW);
//  Serial.println("Stepping\t\tcallback");
}

void serial_callback() {
//  Serial.println("Serial\t\tcallback");
}

void endSwitch_callback() {
  // trigger led toggle on switch high
  switchOneState = digitalRead(END_SWITCH_1);
  switchTwoState = digitalRead(END_SWITCH_2);

  if ((switchOneState == PRESSED_DOWN) || 
      (switchTwoState == PRESSED_DOWN)) {
    motorShouldStop = true;
  }

//  if (!endSwitchOneTriggered && !switchOneState) {
//    shouldToggle = true;
//    endSwitchOneTriggered = true;
//  }
//  
//  if (endSwitchOneTriggered && switchOneState) {
//    endSwitchOneTriggered = false;
//  }

  if (motorShouldStop) {
    stepperOn = false;
    motorShouldStop = false;
  }

  
//  if (shouldToggle) {
//    // toggle light
//    if (ledHigh) {
//      digitalWrite(LED_BUILTIN, LOW);
//      ledHigh = false;
//      stepperOn = false;
//    }
//    else {
//      digitalWrite(LED_BUILTIN, HIGH);
//      ledHigh = true;
//      stepperOn = false;
//    }
//    shouldToggle = false;
//  }
 
//  Serial.println("Endswitch\t\tcallback");
}

// This is the callback for the Timer
void timerCallback(){
  controller.run();
}

void setup() {
  // Stepper motor
//  dirHigh = true;
  stepHigh = false;
  digitalWrite(DIR_PIN, HIGH);
  digitalWrite(STEP_PIN, LOW);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);

  // Built-in LED
  ledHigh = false;
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Endstop(/limit) switch
  pinMode(END_SWITCH_1, INPUT);
  pinMode(END_SWITCH_2, INPUT);
  endSwitchOneTriggered = false;
  endSwitchTwoTriggered = false;

  // Serial
  Serial.begin(9600);
  inputString.reserve(200);

  // Threads
  th_a_step->setInterval(5000/(1.0*x)); // microseconds
  th_a_step->onRun(step_callback);  
  th_s_serial->setInterval(10000); 
  th_s_serial->onRun(serial_callback);  
  th_s_endSwitch->setInterval(10000);
  th_s_endSwitch->onRun(endSwitch_callback);

  // Timer setup
  Timer1.initialize(200);
  Timer1.attachInterrupt(timerCallback);
  Timer1.start();

}

void loop() {
  if (stringComplete) {
    Serial.println(inputString);
    x = inputString.toFloat();
    if (x==0) {
      stepperOn = false;
    }
    else {
      if (x < 0) {
        digitalWrite(DIR_PIN, LOW);
      }
      else {
        digitalWrite(DIR_PIN, HIGH);
      }
      x = abs(x);
      th_a_step->setInterval(5000/(1.0*x));
      stepperOn = true;
    }
    
//    x = inputStringsubstring(1).toFloat();
//    if (inputString.substring(0, 1) == "s") {
//      th_a_step->setInterval(5000/(1.0*x)); // 10 miliseconds
//    }
//    if (inputString.substring(0, 1) == "p") {
//      
//    }
    
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

