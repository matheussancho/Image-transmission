#include <Arduino.h>
#define LED 2

void setup() {
  pinMode(LED, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
    digitalWrite(LED, HIGH);
    delay(1000);
    digitalWrite(LED, LOW);
    delay(1000);
}