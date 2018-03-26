
String inputString = "";


void setup()
{
  // serial
  Serial.begin(9600);
  inputString.reserve(200);
}

void void loop()
{
	// Serial.readBytes(buffer, length);
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    // TODO: change this to Serial.readBytes(buffer, length);
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