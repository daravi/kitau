#include <TimerOne.h>

#include <ThreadController.h>
#include <StaticThreadController.h>
#include <Thread.h>




#define STEP_PIN_1 9
#define DIR_PIN_1 5
#define STEP_PIN_2 10
#define DIR_PIN_2 6
#define END_SWITCH 11
 
//bool dirHigh;
bool stepOneHigh;
bool stepTwoHigh;
bool ledHigh;


bool endSwitchTriggered;
bool shouldToggle;
bool stepperOneOn = false;
bool stepperTwoOn = false;
int switchState = 0;

String inputString = "";         // a String to hold incoming data
boolean stringComplete = false;  // whether the string is complete
float x = 0.0;
float y = 0.0;

Thread* th_a_step_one = new Thread();
Thread* th_a_step_two = new Thread();
Thread* th_s_serial = new Thread();
Thread* th_s_endSwitch = new Thread();

StaticThreadController<3> controller (th_a_step_one, th_a_step_two, th_s_serial, th_s_endSwitch);

void step_one_callback() {
  if (stepOneHigh) {
    digitalWrite(STEP_PIN_1, LOW);
    stepOneHigh = false;
  }
  else {
    if (stepperOneOn) {
      digitalWrite(STEP_PIN_1, HIGH);
      stepOneHigh = true;
    }
  }
//  digitalWrite(STEP_PIN, HIGH);
//  delayMicroseconds(520);
//  digitalWrite(STEP_PIN, LOW);
//  Serial.println("Stepping\t\tcallback");
}

void step_two_callback() {
  if (stepTwoHigh) {
    digitalWrite(STEP_PIN_2, LOW);
    stepTwoHigh = false;
  }
  else {
    if (stepperTwoOn) {
      digitalWrite(STEP_PIN_2, HIGH);
      stepTwoHigh = true;
    }
  }
}

void serial_callback() {
//  Serial.println("Serial\t\tcallback");
}

void endSwitch_callback() {
  // trigger led toggle on switch high
  switchState = digitalRead(END_SWITCH);
  if (!endSwitchTriggered && !switchState) {
    shouldToggle = true;
    endSwitchTriggered = true;
  }
  
  if (endSwitchTriggered && switchState) {
    endSwitchTriggered = false;
  }

  
  if (shouldToggle) {
    // toggle light
    if (ledHigh) {
      digitalWrite(LED_BUILTIN, LOW);
      ledHigh = false;
      stepperOneOn = false;
    }
    else {
      digitalWrite(LED_BUILTIN, HIGH);
      ledHigh = true;
      stepperOneOn = false;
    }
    shouldToggle = false;
  }
 
//  Serial.println("Endswitch\t\tcallback");
}

// This is the callback for the Timer
void timerCallback(){
  controller.run();
}

void setup() {
  // Stepper motor
//  dirHigh = true;
  stepOneHigh = false;
  stepTwoHigh = false;
  digitalWrite(DIR_PIN_1, HIGH);
  digitalWrite(STEP_PIN_1, LOW);
  pinMode(DIR_PIN_1, OUTPUT);
  pinMode(STEP_PIN_1, OUTPUT);

  digitalWrite(DIR_PIN_2, HIGH);
  digitalWrite(STEP_PIN_2, LOW);
  pinMode(DIR_PIN_2, OUTPUT);
  pinMode(STEP_PIN_2, OUTPUT);

  // Built-in LED
  ledHigh = false;
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // Endstop(/limit) switch
  endSwitchTriggered = false;
  shouldToggle = false;
  pinMode(END_SWITCH, INPUT);

  // Serial
  Serial.begin(9600);
  inputString.reserve(200);

  // Threads
  th_a_step_one->setInterval(10000); // microseconds
  th_a_step_one->onRun(step_one_callback);
  th_a_step_two->setInterval(10000); // microseconds
  th_a_step_two->onRun(step_two_callback);
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
    x = inputString.substring(0,1).toFloat();
    y = inputString.substring(1,2).toFloat();
    x = inputString.toFloat();
    
    if (x==0) {
      stepperOneOn = false;
    }
    else {
      if (x < 0) {
        digitalWrite(DIR_PIN_1, LOW);
      }
      else {
        digitalWrite(DIR_PIN_1, HIGH);
      }
      x = abs(x);
      th_a_step_one->setInterval(5000/(1.0*x));
      stepperOneOn = true;
    }

    if (y==0) {
      stepperTwoOn = false;
    }
    else {
      if (y < 0) {
        digitalWrite(DIR_PIN_2, LOW);
      }
      else {
        digitalWrite(DIR_PIN_2, HIGH);
      }
      y = abs(y);
      th_a_step_two->setInterval(5000/(1.0*y));
      stepperTwoOn = true;
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

