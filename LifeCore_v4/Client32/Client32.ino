#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEClient.h>
#include <BLERemoteCharacteristic.h>

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

// PINS
const int trigPin = 2;
const int echoPin = 4;

BLEAddress* pServerAddress = nullptr;
BLERemoteCharacteristic* pRemoteRx = nullptr;
BLEClient* pClient = nullptr;

bool doConnect = false;
bool connected = false;
bool doScan = false; // FIX: flag to trigger re-scan from loop()

unsigned long lastSend = 0;

// --- FIX: Notify callback to receive alerts from S3 ---
void notifyCallback(BLERemoteCharacteristic* pChar, uint8_t* pData, size_t length, bool isNotify) {
  String msg = "";
  for (int i = 0; i < length; i++) msg += (char)pData[i];
  Serial.println("<<< Alert from S3: " + msg);
}

class MyClientCallbacks : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {}
  // FIX: on unexpected disconnect, trigger re-scan
  void onDisconnect(BLEClient* pclient) {
    connected = false;
    doScan = true;
    Serial.println("[!] Disconnected from S3. Will re-scan.");
  }
};

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) override {
    if (advertisedDevice.haveServiceUUID() &&
        advertisedDevice.isAdvertisingService(BLEUUID(SERVICE_UUID))) {
      Serial.println(">>> S3 Server Found!");
      if (pServerAddress != nullptr) delete pServerAddress;
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());
      doConnect = true;
      BLEDevice::getScan()->stop();
    }
  }
};

bool connectToServer() {
  Serial.println("Attempting to connect...");
  if (pClient != nullptr) {
    delete pClient;
    pClient = nullptr;
  }
  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new MyClientCallbacks()); // FIXED: setClientCallbacks is the correct method name

  if (!pClient->connect(*pServerAddress)) {
    Serial.println("Connection failed.");
    return false;
  }

  BLERemoteService* pRemoteService = pClient->getService(BLEUUID(SERVICE_UUID));
  if (pRemoteService == nullptr) {
    Serial.println("Service not found.");
    pClient->disconnect();
    return false;
  }

  pRemoteRx = pRemoteService->getCharacteristic(BLEUUID(CHARACTERISTIC_UUID_RX));
  if (pRemoteRx == nullptr) {
    Serial.println("RX characteristic not found.");
    pClient->disconnect();
    return false;
  }

  // FIX: Subscribe to TX notifications from S3
  BLERemoteCharacteristic* pRemoteTx = pRemoteService->getCharacteristic(BLEUUID(CHARACTERISTIC_UUID_TX));
  if (pRemoteTx != nullptr && pRemoteTx->canNotify()) {
    pRemoteTx->registerForNotify(notifyCallback);
    Serial.println("Subscribed to TX notifications.");
  }

  connected = true;
  return true;
}

void startScan() {
  Serial.println("Scanning for S3...");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.println("Client Starting...");
  BLEDevice::init("");
  startScan();
}

void loop() {
  // FIX: Re-scan if disconnected or triggered
  if (doScan) {
    doScan = false;
    doConnect = false;
    delay(500);
    startScan();
  }

  if (doConnect) {
    if (connectToServer()) Serial.println("Connected Successfully!");
    else {
      Serial.println("Failed to connect. Retrying scan...");
      doScan = true;
    }
    doConnect = false;
  }

  if (millis() - lastSend > 200) {
    lastSend = millis();

    // Trigger ultrasonic
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 30000);
    int distance = (int)(duration * 0.0343 / 2);

    Serial.print("Sensor Read -> Duration: ");
    Serial.print(duration);
    Serial.print(" | Distance: ");
    Serial.print(distance);
    Serial.print("cm");

    // FIX: Send even if distance==0; only skip if duration==0 (no echo / sensor error)
    if (connected && pRemoteRx != nullptr) {
      if (duration > 0) {
        String msg = String(distance);
        pRemoteRx->writeValue((uint8_t*)msg.c_str(), msg.length(), false);
        Serial.println(" [SENT TO S3]");
      } else {
        Serial.println(" [SENSOR ERROR - no echo]");
      }
    } else if (!connected) {
      Serial.println(" [NOT CONNECTED]");
    }
  }
}
