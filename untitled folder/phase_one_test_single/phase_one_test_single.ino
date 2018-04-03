#include <TimerOne.h>

#include <ThreadController.h>
#include <StaticThreadController.h>
#include <Thread.h>

Thread* th_s_endSwitch = new Thread();
Thread* th_s_serial = new Thread();
Thread* th_s_endSwitch = new Thread();

StaticThreadController<3> controller (th_a_step, th_s_serial, th_s_endSwitch);

void step_callback() {
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
      stepperOn = false;
    }
    else {
      digitalWrite(LED_BUILTIN, HIGH);
      ledHigh = true;
      stepperOn = false;
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
  // Threads
  th_a_step->setInterval(10); // microseconds
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
}

