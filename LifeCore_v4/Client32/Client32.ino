#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEClient.h>
#include <BLERemoteCharacteristic.h>

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" 

// PINS: Change these if you moved your wires!
const int trigPin = 2; 
const int echoPin = 4; 

BLEAddress* pServerAddress = nullptr;
BLERemoteCharacteristic* pRemoteRx = nullptr;
BLEClient* pClient = nullptr;
bool doConnect = false;
bool connected = false;
unsigned long lastSend = 0;

bool scanning = false;                              // track if we are currently scanning
unsigned long lastConnectionAttempt = 0;            // prevent spamming reconnect attempts
const unsigned long reconnectInterval = 3000;       // wait 3s between retries

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
      Serial.println(">>> S3 Server Found!");
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      doConnect = true;
      scanning = false;                             // mark scan as done
      BLEDevice::getScan()->stop();
    }
  }
};

// Detect when connection drops
class MyClientCallbacks : public BLEClientCallbacks {
  void onConnect(BLEClient* pClient) {
    Serial.println("[!] Connected to server");
  }
  void onDisconnect(BLEClient* pClient) {
    connected = false;                              // flip flag so loop() knows to reconnect
    Serial.println("[?] Disconnected from server");
  }
};

bool connectToServer() {
  Serial.println("Attempting to connect...");

  // Clean up old client if it exists, cleanup before reconnecting
  if (pClient != nullptr) {
    pClient->disconnect();
    delete pClient;
    pClient = nullptr;
  }

  pClient = BLEDevice::createClient();
  pClient->setCallbacks(new MyClientCallbacks());   // attach disconnect callback

  if (!pClient->connect(*pServerAddress)) return false;
  
  BLERemoteService* pRemoteService = pClient->getService(BLEUUID(SERVICE_UUID));
  if (pRemoteService == nullptr) return false;
  
  pRemoteRx = pRemoteService->getCharacteristic(BLEUUID(CHARACTERISTIC_UUID_RX));
  if (pRemoteRx == nullptr) return false;

  connected = true;
  return true;
}

void startScan() {                        // separated  into a reusable function
  if (!scanning) {
    Serial.println("--- Scanning for server ---");
    scanning = true;
    doConnect = false;
    BLEDevice::getScan()->start(5, false);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  Serial.println("Client Starting...");
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  
  startScan();                            // start initial scan using the reusable function
}

void loop() {
  // Step 1: If scan found the server, connect to it
  if (doConnect) {
    if (connectToServer()) Serial.println("Connected Successfully!");
    else {
        Serial.println("Failed to connect, retrying...");
        lastConnectionAttempt = millis();   // NEW: record failed attempt time
    }    
    
    doConnect = false;
  }

  // Step 2: If not connected and not scanning, decide what to do, reconnection logic
  if (!connected && !scanning) {
    if (millis() - lastConnectionAttempt > reconnectInterval) {
      Serial.println("Not connected — rescanning...");
      startScan();
    }
  }

  // Step 3: Measure and send sensor data
  // --- SENSOR TEST (Always runs) ---
  if (millis() - lastSend > 200) { 
    lastSend = millis();
    
    digitalWrite(trigPin, LOW); delayMicroseconds(2);
    digitalWrite(trigPin, HIGH); delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout
    int distance = duration * 0.0343 / 2;

    // This will print even if Bluetooth is NOT connected
    Serial.print("Sensor Read -> Duration: ");
    Serial.print(duration);
    Serial.print(" | Distance: ");
    Serial.print(distance);

    if (connected && pRemoteRx != nullptr && duration > 0) {
      String msg = String(distance);
      pRemoteRx->writeValue((uint8_t*)msg.c_str(), msg.length(), false);
      Serial.println(" [SENT TO S3]");
    } else if (!connected) {
      Serial.println(" [NOT CONNECTED]");
    } else {
      Serial.println(" [SENSOR ERROR]");
    }
  }
}