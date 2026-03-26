const int trigPin = 11;
const int echoPin = 10;

const int greenLED = 8;
const int yellowLED = 7;
const int redLED = 4;

const int buzzer = 3;
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
  pinMode(potPin, INPUT);		// Probably not needed, but it does not break anything so I'm leaving it
  
  Serial.begin(9600);			// For debugging
}

void loop() {

  // Measure dist
  digitalWrite(trigPin, LOW);     // Clear noise
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);		// Send ultrasonic sound
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.0343 / 2;
  // Sound travels in air at approximately 0.0343 centimeters per microsecond
  // Devide by 2 because sound travels to the object and back (double distance)
  
  // Ignore bad readings (pulseIn returns 0 when reading fails)
  if (duration == 0) return;
  
  // Ignore out of range readings
  if (distance > 400) return;

  // Potentiometer sensitivity logic
  int potValue = analogRead(potPin);	// Reads the potentiometer and returns an int in range (0 - 1023) which is mapped linearly to its voltage
  
  // Map to a sensitivity multiplier which ranges from 0.5 (low) to 2.0 (high)
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

  // Turn off everything
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, LOW);
  noTone(buzzer);
  // delay(10);

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
  }

  // Close — red, fast beep
  else if (distance > closeThreshold)
  {
    digitalWrite(redLED, HIGH);
    tone(buzzer, 800, 100);
  }

  // Critical — all flash, continuous alarm
  else
  {
    digitalWrite(redLED, HIGH);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(greenLED, HIGH);
    tone(buzzer, 1200, 100);
    delay(100);
    digitalWrite(redLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(greenLED, LOW);
    noTone(buzzer);
  }

  delay(100);
}