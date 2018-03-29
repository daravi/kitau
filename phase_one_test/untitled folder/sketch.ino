#include <Crane.h>

// set to a value between 0 and 1
#define X_START 0
#define Y_START 0
#define Z_START 0

Polisort* ps = new Polisort();

void setup() {
	ps->reset(); // reset pos, heater, fan, and UV on
	ps->set_start(X_START, Y_START, Z_START);
}

void loop() {
  if (ps->is_ready()) {
    if (ps->input->is_empty()) {
      ps->stop();
    } else {
      ps->start();
    }
  }
}


reset() {
  heater->on();
  polisher->on();
  crane->reset()
}