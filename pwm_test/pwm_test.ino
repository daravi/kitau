int pwmPin = 9; // connected to the MOSFET's gate
int val = 20;

void setup() {
   pinMode(pwmPin, OUTPUT);
}

void loop() {
   analogWrite(pwmPin, val);
}