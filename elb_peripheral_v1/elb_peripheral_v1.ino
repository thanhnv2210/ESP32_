#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <time.h>
#include <WiFi.h>
#include <BLE2902.h>  // Ensure this is included

#define SERVICE_UUID        "12345678-1234-5678-1234-123456789abc"
#define CHARACTERISTIC_UUID "abcdefab-1234-5678-1234-abcdefabcdef"

const char* ssid = "SINGTEL-WNP6";        // Replace with your Wi-Fi
const char* password = "thanhxuan";

BLECharacteristic *pTimeCharacteristic;
bool deviceConnected = false;

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device connected");
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected");
  }
};

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Configure time with NTP
  // Adjust timezone offset as needed (seconds)
  configTime(0, 0, "pool.ntp.org");
  Serial.println("Waiting for time");
  struct tm timeinfo;
  int retries = 0;
  while (!getLocalTime(&timeinfo) && retries < 10) {
    delay(1000);
    retries++;
  }
  if (retries >= 10) {
    Serial.println("Failed to get time");
  } else {
    Serial.println("Time acquired");
  }

  // Initialize BLE after time is loaded
  BLEDevice::init("ESP32_TimeSender");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create characteristic with notify
  pTimeCharacteristic = pService->createCharacteristic(
                            CHARACTERISTIC_UUID,
                            BLECharacteristic::PROPERTY_NOTIFY
                         );
  pTimeCharacteristic->addDescriptor(new BLE2902());

  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE advertising started");
}

void loop() {
  if (deviceConnected) {
    // Send current epoch time
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      time_t nowTime = time(NULL);
      String timeStr = String(nowTime);
      pTimeCharacteristic->setValue(timeStr.c_str());
      pTimeCharacteristic->notify();
      Serial.println("Sent time: " + timeStr);
    } else {
      Serial.println("Time not available");
    }
  }
  delay(10000); // send every 10 seconds
}