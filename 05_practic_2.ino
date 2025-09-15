int ledPin = 7;

void setup() {
  pinMode(ledPin, OUTPUT);
}

void loop() {
  digitalWrite(ledPin, LOW);
  delay(1000);

  for (int i = 0; i < 5; i++) {
    digitalWrite(ledPin, HIGH);
    delay(100);  
    digitalWrite(ledPin, LOW);
    delay(100);  
  }

  digitalWrite(ledPin, HIGH);

  while (1);
}
