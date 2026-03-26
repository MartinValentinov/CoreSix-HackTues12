#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// --- BLE CONFIG ---
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" 

// --- PERIPHERAL PINS ---
const int trigPin = 10;
const int echoPin = 11;
const int greenLED = 8;
const int yellowLED = 7;
const int redLED = 15;
const int buzzer = 16;
const int potPin = 4; 

// --- VARIABLES ---
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
unsigned long lastValidReadTime = 0;
const unsigned long timeoutInterval = 500; 

// BLE Callbacks
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("\n[!] КЛИЕНТ СЕ СВЪРЗА");
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("\n[?] КЛИЕНТ ПРЕКЪСНА ВРЪЗКАТА");
    }
};

class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String rxValue = pCharacteristic->getValue();
      if (rxValue.length() > 0) {
        Serial.print("-> Получено от клиента: ");
        Serial.println(rxValue);
      }
    }
};

void setup() {
  Serial.begin(115200);
  // S3 Fix for Serial
  unsigned long start = millis();
  while(!Serial && millis() - start < 3000); 

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(buzzer, OUTPUT);

  Serial.println("\n--- СИСТЕМАТА СТАРТИРА ---");

  BLEDevice::init("ESP32-S3-Server");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pTxCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_TX,
                        BLECharacteristic::PROPERTY_NOTIFY
                      );
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
                                           CHARACTERISTIC_UUID_RX,
                                           BLECharacteristic::PROPERTY_WRITE
                                         );
  pRxCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  pService->start();
  BLEDevice::getAdvertising()->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();
  
  Serial.println("--- BLE ADVERTISING STARTED ---");
}

void loop() {
  // 1. Мерим разстояние
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH, 25000); 
  int distance = duration * 0.0343 / 2;

  // 2. Четем потенциометъра
  int potValue = analogRead(potPin);
  float sensitivity = map(potValue, 0, 4095, 50, 200) / 100.0;

  // 3. ПРИНТОВЕ ЗА ДЕБЪГ (Винаги се виждат)
  Serial.print("D: "); Serial.print(distance); Serial.print("cm");
  Serial.print(" | Pot: "); Serial.print(potValue);
  Serial.print(" | Sens: "); Serial.print(sensitivity);
  
  // 4. Логика за Timeout и Светофара
  if (duration == 0 || distance > 400 || distance <= 2) {
    if (millis() - lastValidReadTime > timeoutInterval) {
      Serial.println(" | [TIMEOUT - STAY GREEN]");
      digitalWrite(greenLED, HIGH);
      digitalWrite(yellowLED, LOW);
      digitalWrite(redLED, LOW);
      noTone(buzzer);
    } else {
      Serial.println(" | [SKIP]");
    }
  } else {
    lastValidReadTime = millis();
    updateHardware(distance, sensitivity);
    Serial.println(" | [OK]");
    
    // 5. Пращаме по Bluetooth
    if (deviceConnected) {
      String dataStr = String(distance);
      pTxCharacteristic->setValue((uint8_t*)dataStr.c_str(), dataStr.length());
      pTxCharacteristic->notify();
    }
  }

  // Рестарт на рекламата при дисконект
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); 
    pServer->startAdvertising();
    Serial.println("--- Advertising Restarted ---");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }

  delay(50); // Бърз лууп
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
    // Критично близо
    digitalWrite(redLED, HIGH); digitalWrite(yellowLED, HIGH); digitalWrite(greenLED, HIGH);
    tone(buzzer, 1200, 60);
  }
}