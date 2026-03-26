const int trigPin = 9;
const int echoPin = 10;

const int greenLED = 2;
const int yellowLED = 3;
const int redLED = 4;

const int buzzer = 11;
const int potPin = A0;        // Potentiometer analog input

long duration;
int distance;

void setup() {
  pinMode(trigPin, OUTPUT);		// Sends signal
  pinMode(echoPin, INPUT);		// Receives signal
  
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  
  pinMode(buzzer, OUTPUT);
  pinMode(potPin, INPUT);		// Probably not needed, but it does not break anything 
  
  Serial.begin(9600);			// For debugging
}

void loop() {
  int potValue = analogRead(potPin);	// Reads the potentiometer and returns an int in range (0 - 1023)
  
  // Map to a sensitivity multiplier, sensitivity ranges from 0.5 (low) to 2.0 (high)
  float sensitivity = map(potValue, 0, 1023, 50, 200) / 100.0;	

  // Calculate thresholds
  int farThreshold      = 100 * sensitivity;	// default 100cm
  int mediumThreshold   = 50  * sensitivity;	// default 50cm
  int closeThreshold    = 20  * sensitivity;	// default 20cm

  // Debug
  Serial.print("Sensitivity: ");
  Serial.print(sensitivity);
  Serial.print(" | Thresholds: ");
  Serial.print(farThreshold);
  Serial.print(" / ");
  Serial.print(mediumThreshold);
  Serial.print(" / ");
  Serial.print(closeThreshold);
  Serial.print(" | Distance: ");
  Serial.println(distance);

  // Measure dist
  digitalWrite(trigPin, LOW);		// Clear noise
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);		// Send ultrasonic sound
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.0343 / 2;
  // Sound travels in air at approximately 0.0343 centimeters per microsecond
  // Devide by 2 sound travels to the object and back (double distance)
  
  // Ignore bad readings (pulseIn returns 0 when reading fails)
  if (duration == 0) return;
  
  // Ignore out of range readings
  if (distance > 400) return;

  // Turn off everything
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, LOW);
  noTone(buzzer);

  // Far — green, no sound
  if (distance > farThreshold)
  {
    digitalWrite(greenLED, HIGH);
    delay(200);
  }

  // Medium — yellow, slow beep
  else if (distance > mediumThreshold)
  {
    digitalWrite(yellowLED, HIGH);
    tone(buzzer, 500, 100);
    delay(200);
  }

  // Close — red, fast beep
  else if (distance > closeThreshold)
  {
    digitalWrite(redLED, HIGH);
    tone(buzzer, 800, 100);
    delay(200);
  }

  // Critical — all flash, continuous alarm
  else
  {
    digitalWrite(redLED, HIGH);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(greenLED, HIGH);
    tone(buzzer, 1200);
    delay(100);
    digitalWrite(redLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(greenLED, LOW);
    delay(100);
  }
}