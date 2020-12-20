#include <Arduino.h>
#include <NimBLEDevice.h>

class ClientCallbacks : public NimBLEClientCallbacks
{
  void onConnect(NimBLEClient *pClient)
  {
    Serial.println("Connected");
    /** After connection we should change the parameters if we don't need fast response times.
         *  These settings are 150ms interval, 0 latency, 450ms timout. 
         *  Timeout should be a multiple of the interval, minimum is 100ms.
         *  I find a multiple of 3-5 * the interval works best for quick response/reconnect.
         *  Min interval: 120 * 1.25ms = 150, Max interval: 120 * 1.25ms = 150, 0 latency, 60 * 10ms = 600ms timeout 
         */
    pClient->updateConnParams(120, 120, 0, 60);
  };

  /** Called when the peripheral requests a change to the connection parameters.
     *  Return true to accept and apply them or false to reject and keep 
     *  the currently used parameters. Default will return true.
     */
  bool onConnParamsUpdateRequest(NimBLEClient *pClient, const ble_gap_upd_params *params)
  {
    if (params->itvl_min < 24)
    { /** 1.25ms units */
      return false;
    }
    else if (params->itvl_max > 40)
    { /** 1.25ms units */
      return false;
    }
    else if (params->latency > 2)
    { /** Number of intervals allowed to skip */
      return false;
    }
    else if (params->supervision_timeout > 100)
    { /** 10ms units */
      return false;
    }

    return true;
  };

  /********************* Security handled here **********************
    ****** Note: these are the same return values as defaults ********/
  uint32_t onPassKeyRequest()
  {
    Serial.println("Client Passkey Request");
    /** return the passkey to send to the server */
    return 123356;
  };

  bool onConfirmPIN(uint32_t pass_key)
  {
    Serial.print("The passkey YES/NO number: ");
    Serial.println(pass_key);
    /** Return false if passkeys don't match. */
    return true;
  };

  /** Pairing process complete, we can check the results in ble_gap_conn_desc */
  void onAuthenticationComplete(ble_gap_conn_desc *desc)
  {
    if (!desc->sec_state.encrypted)
    {
      Serial.println("Encrypt connection failed - disconnecting");
      /** Find the client with the connection handle provided in desc */
      NimBLEDevice::getClientByID(desc->conn_handle)->disconnect();
      return;
    }
  };
};
static ClientCallbacks clientCB;

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
      pClient->setClientCallbacks(&clientCB, false);

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