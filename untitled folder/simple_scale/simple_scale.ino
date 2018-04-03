#include <hx711.h>

Hx711 scale(A2, A3);

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.print(scale.getGram(), 1);
  Serial.println(" g");
  delay(200);
}
