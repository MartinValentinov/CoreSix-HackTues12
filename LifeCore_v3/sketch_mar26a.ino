#include <Arduino.h>

const int trigPin = 10;
const int echoPin = 11;
const int greenLED = 8;
const int yellowLED = 7;
const int redLED = 15;
const int buzzer = 16;
const int potPin = 4; 

unsigned long lastValidReadTime = 0;
const unsigned long timeoutInterval = 500; // 0.5 seconds timeout (Much faster!)

void setup() {
  // IMPORTANT: For S3, 115200 is standard. 
  // Make sure Serial Monitor matches this!
  Serial.begin(115200);
  
  // Wait for Serial to initialize (S3 specific trick)
  while(!Serial && millis() < 3000); 

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(buzzer, OUTPUT);

  Serial.println("\n--- S3 DIAGNOSTIC START ---");
}

void loop() {
  // 1. Trigger the sensor
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // 2. Read the pulse (with a short 20ms internal timeout)
  long duration = pulseIn(echoPin, HIGH, 20000); 
  int distance = duration * 0.0343 / 2;

  // 3. Print RAW data every loop so you see movement in the monitor
  Serial.print("Raw Duration: ");
  Serial.print(duration);
  Serial.print(" | ");

  // 4. Check for valid data
  if (duration == 0 || distance > 400 || distance <= 2) {
    // If we are in a silence period longer than 500ms...
    if (millis() - lastValidReadTime > timeoutInterval) {
      Serial.println("STATE: TIMEOUT (STAYING GREEN)");
      digitalWrite(greenLED, HIGH);
      digitalWrite(yellowLED, LOW);
      digitalWrite(redLED, LOW);
      noTone(buzzer);
    } else {
      Serial.println("STATE: DROPPED PACKET (WAITING)");
    }
  } else {
    // Valid data received!
    lastValidReadTime = millis();
    
    // Potentiometer Check
    int potValue = analogRead(potPin);
    float sensitivity = map(potValue, 0, 4095, 50, 200) / 100.0;
    
    Serial.print("Dist: ");
    Serial.print(distance);
    Serial.print("cm | Sens: ");
    Serial.println(sensitivity);

    updateLEDs(distance, sensitivity);
  }

  delay(50); // Fast 20Hz refresh rate
}

void updateLEDs(int dist, float sens) {
  int far = 100 * sens;
  int med = 50 * sens;
  int close = 20 * sens;

  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, LOW);

  if (dist > far) {
    digitalWrite(greenLED, HIGH);
    noTone(buzzer);
  } 
  else if (dist > med) {
    digitalWrite(yellowLED, HIGH);
    tone(buzzer, 500, 20);
  } 
  else if (dist > close) {
    digitalWrite(redLED, HIGH);
    tone(buzzer, 800, 20);
  } 
  else {
    digitalWrite(redLED, HIGH);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(greenLED, HIGH);
    tone(buzzer, 1200, 50);
  }
}