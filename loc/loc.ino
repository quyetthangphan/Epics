int FAN = 26;
void setup() {
  Serial.begin(115200);
  pinMode(FAN, OUTPUT);
  digitalWrite(FAN, LOW);
}

void loop() {
  if (Serial.available() > 0) {
    char comandCharacter = Serial.read();
    if (comandCharacter == 'S') {
      digitalWrite(FAN, HIGH);
      Serial.println("FAN ON");
    }
    else
    {
      digitalWrite(FAN, LOW);
      Serial.println("FAN OFF");
    }
  }
}
