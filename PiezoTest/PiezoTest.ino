#include <Tone.h>

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 
Tone myTone;

void setup() {                
  // initialize the digital pin as an output.
  // Pin 13 has an LED connected on most Arduino boards:
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(13, OUTPUT);
  
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  
  myTone.begin(3, 4);
  myTone.play(4800);
}

void loop() {
  digitalWrite(13, HIGH);
  delay(125);
  digitalWrite(13, LOW);
  delay(125);
  digitalWrite(13, HIGH);
  delay(125);
  digitalWrite(13, LOW);
  delay(125);
  digitalWrite(13, HIGH);
  delay(125);
  digitalWrite(13, LOW);
  delay(1000);
}
