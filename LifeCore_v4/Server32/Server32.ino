#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <WiFi.h>
#include <HTTPClient.h>

// --- BLE CONFIG ---
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

BLECharacteristic *pTxCharacteristic;

// --- PERIPHERAL PINS ---
const int greenLED  = 8;
const int yellowLED = 7;
const int redLED    = 15;
const int buzzer    = 16;
const int potPin    = 4;

// --- STATE ---
BLEServer *pServer = NULL;
bool deviceConnected    = false;
bool oldDeviceConnected = false;

// FIX: Use a mutex to safely share data between BLE callback (Core 1) and postTask (Core 0)
portMUX_TYPE dataMux = portMUX_INITIALIZER_UNLOCKED;
int  sharedDistance  = 999;
unsigned long sharedLastDataTime = 0;

// Local copies used in loop() and postTask — updated under mutex
int  receivedDistance = 999;
unsigned long lastDataTime = 0;

// Shared sensitivity so postTask uses the same scaled thresholds as updateHardware()
float sharedSensitivity = 1.0f;

unsigned long lastVoiceNotifyTime = 0;
int lastAlertLevel = 0;

// --- WIFI CONFIG ---
const char* ssid      = "";
const char* password  = "";
const char* serverUrl = "";

// --- FUNCTION PROTOTYPES ---
void updateHardware(int dist, float sens);
void postTask(void* parameter);

// --- BLE CALLBACKS ---
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("[!] Client connected");
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("[?] Client disconnected");
  }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    String rxValue = pCharacteristic->getValue();
    if (rxValue.length() > 0) {
      int dist = rxValue.toInt();
      // FIX: Write shared data under mutex
      portENTER_CRITICAL(&dataMux);
      sharedDistance     = dist;
      sharedLastDataTime = millis();
      portEXIT_CRITICAL(&dataMux);
      Serial.print("-> Distance received: ");
      Serial.println(dist);
    }
  }
};

void setup() {
  Serial.begin(115200);

  pinMode(greenLED,  OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED,    OUTPUT);
  pinMode(buzzer,    OUTPUT);

  // Safe startup state
  digitalWrite(greenLED,  HIGH); // Green = waiting
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED,    LOW);
  noTone(buzzer);

  // --- WiFi --- FIX: Set mode FIRST before begin, avoids "cannot set config" error
  // WiFi and BLE share the RF module on ESP32-S3; mode must be set before BLE init
  WiFi.mode(WIFI_STA);
  delay(100);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  int wifiRetries = 0;
  while (WiFi.status() != WL_CONNECTED && wifiRetries < 20) {
    delay(500);
    Serial.print(".");
    wifiRetries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nWiFi FAILED — continuing without it.");
  }

  // FIX: Start POST task on Core 0 AFTER WiFi is up
  xTaskCreatePinnedToCore(
    postTask,
    "PostTask",
    8192,
    NULL,
    1,
    NULL,
    0  // Core 0
  );

  // --- BLE on Core 1 (default) ---
  BLEDevice::init("ESP32-S3-Server");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService* pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_RX,
    BLECharacteristic::PROPERTY_WRITE
  );
  pRxCharacteristic->setCallbacks(new MyCharacteristicCallbacks());

  pTxCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID_TX,
    BLECharacteristic::PROPERTY_NOTIFY
  );
  pTxCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  BLEDevice::getAdvertising()->addServiceUUID(SERVICE_UUID);
  BLEDevice::startAdvertising();

  Serial.println("--- BLE Advertising. Waiting for client ---");
}

void loop() {
  static unsigned long lastLoopTime = 0;
  unsigned long now = millis();
  if (now - lastLoopTime < 10) return;
  lastLoopTime = now;

  // FIX: Safely read shared data into local copies
  portENTER_CRITICAL(&dataMux);
  receivedDistance = sharedDistance;
  lastDataTime     = sharedLastDataTime;
  portEXIT_CRITICAL(&dataMux);

  // Potentiometer sensitivity (0.5 – 2.0)
  // FIX: map() does integer math internally — cast to float BEFORE dividing
  int potValue = analogRead(potPin);
  float sensitivity = (float)map(potValue, 0, 4095, 50, 200) / 100.0f;
  sharedSensitivity = sensitivity; // share with postTask

  // FIX: Timeout = no data for >1 second → idle state
  if (now - lastDataTime > 1000) {
    digitalWrite(greenLED,  HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED,    LOW);
    noTone(buzzer);
  } else {
    updateHardware(receivedDistance, sensitivity);
  }

  // BLE reconnection
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);
    pServer->startAdvertising();
    Serial.println("Restarted advertising.");
    oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = deviceConnected;
  }
}

// --- Hardware logic ---
void updateHardware(int dist, float sens) {
  int farThresh   = (int)(100 * sens);
  int medThresh   = (int)(50  * sens);
  int closeThresh = (int)(20  * sens);

  digitalWrite(greenLED,  LOW);
  digitalWrite(yellowLED, LOW);
  digitalWrite(redLED,    LOW);

  // FIX: tone() has no volume control — perceived loudness is shaped by
  // frequency + pulse duration + how often it fires.
  // Yellow: low freq, very short blip, fires rarely = sounds much softer
  // Red: medium freq, normal pulse
  // Critical: high freq, long pulse = clearly loudest
  static unsigned long lastYellowBeep = 0;
  unsigned long nowBeep = millis();

  if (dist > farThresh) {
    digitalWrite(greenLED, HIGH);
    noTone(buzzer);
  } else if (dist > medThresh) {
    digitalWrite(yellowLED, HIGH);
    if (nowBeep - lastYellowBeep > 600) { // only beep every 600ms
      tone(buzzer, 400, 8);               // 400Hz, 8ms = very soft blip
      lastYellowBeep = nowBeep;
    }
  } else if (dist > closeThresh) {
    digitalWrite(redLED, HIGH);
    tone(buzzer, 800, 30);
  } else {
    // Critical — all LEDs + fast tone
    digitalWrite(greenLED,  HIGH);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(redLED,    HIGH);
    tone(buzzer, 1200, 60);
  }

  int    currentAlertLevel = 0;
  String message = "";

  if (dist <= closeThresh)      { currentAlertLevel = 2; message = "Warning!"; }
  else if (dist <= medThresh)   { currentAlertLevel = 1; message = "Caution!"; }

  if (deviceConnected && currentAlertLevel > 0) {
    unsigned long now = millis();
    if (currentAlertLevel != lastAlertLevel || (now - lastVoiceNotifyTime > 3000)) {
      pTxCharacteristic->setValue(message.c_str());
      pTxCharacteristic->notify();
      lastVoiceNotifyTime = now;
      lastAlertLevel = currentAlertLevel;
    }
  }
  if (currentAlertLevel == 0) lastAlertLevel = 0;
}

// --- HTTP POST task (Core 0) ---
void postTask(void* parameter) {
  for (;;) {
    // FIX: Read shared data safely inside the task too
    portENTER_CRITICAL(&dataMux);
    int  dist     = sharedDistance;
    unsigned long ldt = sharedLastDataTime;
    portEXIT_CRITICAL(&dataMux);

    if (WiFi.status() == WL_CONNECTED && (millis() - ldt) <= 2000) {
      HTTPClient http;
      http.begin(serverUrl);
      http.addHeader("Content-Type", "application/json");
      http.setTimeout(3000);

      // Use the same sensitivity-scaled thresholds as updateHardware()
      float sens = sharedSensitivity;
      int farThresh   = (int)(100 * sens);
      int medThresh   = (int)(50  * sens);
      int closeThresh = (int)(20  * sens);

      String alert;
      if (dist <= closeThresh)     alert = "WARNING! " + String(dist) + "cm ahead - Stop immediately!";
      else if (dist <= medThresh)  alert = "Caution! " + String(dist) + "cm ahead";
      else if (dist <= farThresh)  alert = "Approaching object, " + String(dist) + "cm ahead";
      else                         alert = "Clear, " + String(dist) + "cm";

      String payload = "{\"distance\":" + String(dist) + ",\"alert\":\"" + alert + "\"}";
      int code = http.POST(payload);

      if (code > 0) {
        String resp = http.getString();
        Serial.println("POST " + String(code) + ": " + resp);
      } else {
        Serial.println("POST ERROR: " + http.errorToString(code));
      }
      http.end();
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
