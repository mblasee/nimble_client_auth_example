#include <Arduino.h>
#include <NimBLEDevice.h>

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting NimBLE Client");

  NimBLEDevice::init("");
  NimBLEDevice::setSecurityAuth(true, true, true);
  NimBLEScan *pScan = NimBLEDevice::getScan();
  NimBLEScanResults results = pScan->start(5);

  NimBLEUUID serviceUuid("ABCD");

  for (int i = 0; i < results.getCount(); i++)
  {
    NimBLEAdvertisedDevice device = results.getDevice(i);
    Serial.println(device.getName().c_str());

    if (device.isAdvertisingService(serviceUuid))
    {
      NimBLEClient *pClient = NimBLEDevice::createClient();

      if (pClient->connect(&device))
      {
        pClient->secureConnection();
        NimBLERemoteService *pService = pClient->getService(serviceUuid);
        if (pService != nullptr)
        {
          NimBLERemoteCharacteristic *pCharacteristic = pService->getCharacteristic("1234");

          if (pCharacteristic != nullptr)
          {
            std::string value = pCharacteristic->readValue();
            // print or do whatever you need with the value
            Serial.println(value.c_str());
          }
        }
      }
      else
      {
        // failed to connect
        Serial.println("failed to connect");
      }

      NimBLEDevice::deleteClient(pClient);
    }
  }
}

void loop()
{
}