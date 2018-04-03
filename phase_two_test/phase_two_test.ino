#include <Q2HX711.h>

#define HX711_DAT_PIN 3
#define HX711_CLK_PIN 2

Q2HX711 load_cell(HX711_DAT_PIN, HX711_CLK_PIN);

void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println(load_cell.read()/100.0);
  delay(500);
}
