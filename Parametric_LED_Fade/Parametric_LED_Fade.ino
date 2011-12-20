/*
 Fade
 
 This example shows how to fade an LED on pin 9
 using the analogWrite() function.
 
 This example code is in the public domain.
 
 */
long fadeDurationMicroseconds = 1000 * 1000L;
int maxBrightness = 64;
int minBrightness = 0;

long currentFadeCycleStartMicroseconds = 0;
int direction = 1;

void setup()  {
  Serial.begin(9600);
  // declare pin 9 to be an output:
  pinMode(9, OUTPUT);
  
  currentFadeCycleStartMicroseconds = micros();
} 

void loop()  { 
    long cur = micros();
    long delta = cur - currentFadeCycleStartMicroseconds;
  
  // If the current fade cycle has completed, start a new one.
  if (delta > fadeDurationMicroseconds)
  {
    Serial.println(direction);
    currentFadeCycleStartMicroseconds = cur;
    delta = 0;
    direction = -direction;
  }
  
  double cycleCompletePercentage = ((double)delta) / fadeDurationMicroseconds;
  if (direction < 0)
  {
    cycleCompletePercentage = 1.0 - cycleCompletePercentage;
  }
  int brightness = minBrightness + ((maxBrightness - minBrightness) * cycleCompletePercentage);

  // set the brightness of pin 9:
  analogWrite(9, brightness);
}

