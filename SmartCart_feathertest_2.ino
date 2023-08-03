#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Adafruit_NeoPixel.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define PIN 8
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// define the pwm pin
#define PWM_PIN 12

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      for (int i = 0; i < 10; i++) {  // Blink 10 times when connected
        pixels.setPixelColor(0, pixels.Color(0, 0, 255)); // Set to blue
        pixels.show(); // Apply changes
        delay(500); // Wait for half a second
        pixels.clear(); // Turn off
        pixels.show(); // Apply changes
        delay(500); // Wait for half a second
      }
    }

    void onDisconnect(BLEServer* pServer) {
      pixels.setPixelColor(0, pixels.Color(0, 0, 255)); // Set to blue
      pixels.show(); // Apply changes
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() == 2) { // Make sure that two bytes were received
        int x = (unsigned char) rxValue[0] - 128; // Cast byte to int and shift range back to -128~127
        int y = (unsigned char) rxValue[1] - 128; // Cast byte to int and shift range back to -128~127

        Serial.print(x);
        Serial.print(",");
        Serial.println(y);

        int absY = abs(y) * 2; // Get absolute Y-value and map to 0-255
        analogWrite(PWM_PIN, absY); // Write PWM signal to pin

        // Put your joystick handling code here

      } else {
        // The received value wasn't two bytes long. Handle this error.
      }
    }
};

void setup() {
  Serial.begin(115200);
  
  // Create the BLE Device
  BLEDevice::init("SmartCart");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();

  // Set server callbacks
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ   |
                                         BLECharacteristic::PROPERTY_WRITE  |
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_INDICATE
                                       );

  pCharacteristic->addDescriptor(new BLE2902());

  // Set the value of the characteristic
  pCharacteristic->setValue("Hello, World!");

  // Set callback for characteristic
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  BLEDevice::startAdvertising();

  // Initialize the NeoPixel
  pixels.begin(); 
  pixels.setPixelColor(0, pixels.Color(0, 0, 255)); // Set to blue
  pixels.show(); // Apply changes
  
  // Setup the PWM pin
  pinMode(PWM_PIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(2000); // We delay here for demo purposes
}
