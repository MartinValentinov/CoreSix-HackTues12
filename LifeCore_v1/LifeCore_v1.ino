const int trigPin = 9;
const int echoPin = 10;

const int greenLED = 2;
const int yellowLED = 3;
const int redLED = 4;

const int buzzer = 11;

long duration;
int distance;

void setup() {
  pinMode(trigPin, OUTPUT);		// Sends signal
  pinMode(echoPin, INPUT);		// Resieves signal
  
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  
  pinMode(buzzer, OUTPUT);
  
  Serial.begin(9600);			// For debugging
}

void loop() {
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

  if (duration == 0) return; // pulseIn returns 0 whan reading fails so we skip it with return
  
  Serial.print("Distance: ");
  Serial.println(distance);

  // Turn off everything
  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, LOW);
  noTone(buzzer);

  // Far — green, no sound
  if (distance > 200)
  {  
    digitalWrite(greenLED, HIGH);
    delay(200);
  }
  
  // Medium — yellow, slow beep
  else if (distance > 100) 
  {
    digitalWrite(yellowLED, HIGH);
    tone(buzzer, 500, 100);			// plays a 500Hz beep for 100ms
    delay(200);
  }
  
  // Close — red, fast beep
  else if (distance > 50) 
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