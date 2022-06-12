#include <RadioLib.h>

#include <list>

#include "Payload.h"

class LoRaTask {
  PhysicalLayer* phyLayer;

  const uint TXDelay;
  unsigned long TXStart = 0, TXEnd = 0;
  const bool AddCR;

 protected:
  bool enableRX;
  bool enableTX;
  uint dutyPercent;

  unsigned long RXEnd = 0, RXCarrierDetected = 0;

  int transmissionState = RADIOLIB_ERR_NONE;

  virtual bool carrierDetected() const = 0;
  virtual bool validHeaderDetected() const = 0;
  virtual bool startReceive() const = 0;

 protected:
  float bw;
  uint8_t sf;
  uint8_t cr;

 public:
  LoRaTask() = delete;
  LoRaTask(bool enableRX = true, bool enableTX = true, const uint txDelay = 1000, const bool addCR = false)
      : enableRX(enableRX), enableTX(enableTX), TXDelay(txDelay), AddCR(addCR){};

  std::list<Payload> AX25UI_TXQueue;
  std::list<Payload> AX25UI_RXQueue;

  bool setup(PhysicalLayer* pl) {
    phyLayer = pl;
    RXEnd = millis() + 5000;  // to work TXDelay when boot.
    Serial.print(F("Data Rate: "));
    Serial.print(bw * sf * 4000 / (cr << sf));
    Serial.println(F("bps."));
    return true;
  }

  bool TXloop() {
    if (!enableTX) {
      while (!AX25UI_TXQueue.empty()) AX25UI_TXQueue.pop_front();
    } else {
      if (TXEnd - TXStart >= 0 /* transmit completed */ && RXCarrierDetected == 0) {
        unsigned long now = millis();
        if (now - RXEnd > TXDelay) {
          if (now - TXEnd > (100 / dutyPercent - 1) * (TXEnd - TXStart)) {
            if (!AX25UI_TXQueue.empty()) {
              if (carrierDetected()) {  // Carrier detected.
                if (validHeaderDetected()) {
                  RXCarrierDetected = millis();
                }
              } else {
                auto xmit = AX25UI_TXQueue.front();
                Serial.print(F("LORA<-:"));
                Serial.println(xmit.toString());
                size_t len = xmit.getLen();
                uint8_t data[256];
                data[0] = '<';
                data[1] = '\xff';
                data[2] = '\x01';
                memcpy(data + 3, xmit.getData(), len);
                len += 3;
                if (AddCR && data[len - 1] != '\r') {
                  data[len] = '\r';
                  len++;
                }

                transmissionState = phyLayer->startTransmit(data, len);
                TXStart = millis();
                AX25UI_TXQueue.pop_front();
              }
            }
          }
        }
      }
    }
    return true;
  }

  bool TXDone() {
    TXEnd = millis();

    if (transmissionState != RADIOLIB_ERR_NONE) {
      Serial.print(F("LoRa TX failed, code "));
      Serial.println(transmissionState);
      return true;
    }

    Serial.println(F("LoRa TX Done."));

    return startReceive();
  }
};