#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// --- BLE CONFIG ---
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" 

// --- PERIPHERAL PINS ---
const int greenLED = 8;
const int yellowLED = 7;
const int redLED = 15;
const int buzzer = 16;
const int potPin = 4; 

// --- VARIABLES ---
BLEServer *pServer = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;

// We store the distance received from the other ESP32 here
int receivedDistance = 999; 
unsigned long lastDataTime = 0;
const unsigned long timeoutInterval = 1000; // 1 second timeout

// Function prototype so the callback can see it
void updateHardware(int dist, float sens);

// BLE Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("[!] Client is connected");
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("[?] Client disconnected");
    }
};

class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        // Convert the string received from the other ESP32 to an integer
        receivedDistance = rxValue.toInt();
        lastDataTime = millis(); 
        
        Serial.print("-> Client says distance is: ");
        Serial.println(receivedDistance);
      }
    }
};

void setup() {
  Serial.begin(115200);
  unsigned long start = millis();
  while(!Serial && millis() - start < 3000); 

  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(buzzer, OUTPUT);

  Serial.println("\n--- S3 Receiver System Starting ---");

  BLEDevice::init("ESP32-S3-Server");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE
                                         );
  pRxCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  pService->start();
  BLEDevice::getAdvertising()->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();
  
  Serial.println("--- Waiting for Client Data ---");
}

void loop() {
  // 1. Read local Potentiometer for sensitivity
  int potValue = analogRead(potPin);
  float sensitivity = map(potValue, 0, 4095, 50, 200) / 100.0;

  // 2. Check if we haven't heard from the client in a while (Timeout)
  if (millis() - lastDataTime > timeoutInterval) {
    // Default state: Green LED Only
    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
    noTone(buzzer);
  } else {
    // We have fresh data! Update LEDs and Buzzer
    updateHardware(receivedDistance, sensitivity);
  }

  // Handle Bluetooth reconnection
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); 
    pServer->startAdvertising();
    Serial.println("--- Restarting Advertising ---");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  delay(50); 
}

void updateHardware(int dist, float sens) {
  int far = 100 * sens;
  int med = 50 * sens;
  int close = 20 * sens;

  digitalWrite(greenLED, LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED, LOW);

  if (dist > far) {
    digitalWrite(greenLED, HIGH);
    noTone(buzzer);
  } else if (dist > med) {
    digitalWrite(yellowLED, HIGH);
    tone(buzzer, 500, 30);
  } else if (dist > close) {
    digitalWrite(redLED, HIGH);
    tone(buzzer, 800, 30);
  } else {
    digitalWrite(redLED, HIGH); 
    digitalWrite(yellowLED, HIGH); 
    digitalWrite(greenLED, HIGH);
    tone(buzzer, 1200, 60);
  }
}