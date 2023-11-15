#include "LoRaTask.h"

bool LoRaTask::setup(PhysicalLayer* pl) {
  phyLayer = pl;
  RXEnd = millis();  // to work TXDelay when boot.
  Serial.print(F("Data Rate: "));
  Serial.print(bw * sf * 4000 / (cr << sf));
  Serial.println(F("bps."));
  return true;
}

bool LoRaTask::taskTX(portTickType xBlockTime) {
  bool isDo = false;

  if (!enableTX) {
    Payload xmit;
    while (xQueueReceive(AX25UI_TXQ, &xmit, xBlockTime) == pdTRUE)
      ;
  } else {
    if (!nowTX && RXCarrierDetected == 0 && TXEnd - TXStart >= 0 /* transmit completed */) {
      int delay = (100 / (float)dutyPercent - 1) * (TXEnd - TXStart) - (millis() - TXEnd);
      if (delay > 0) {
        vTaskDelay(pdMS_TO_TICKS(delay));
      }

      Payload xmit;
      if (xQueuePeek(AX25UI_TXQ, &xmit, xBlockTime) == pdTRUE) {
        Serial.print(F("LORA<-: "));
        Serial.println(xmit.toString());

        int delay = TXDelay - (uint)(millis() - RXEnd);
        if (delay > 0) {
          vTaskDelay(pdMS_TO_TICKS(delay));
        }

        while (carrierDetected()) {  // Carrier detected.
          if (validHeaderDetected()) {
            RXCarrierDetected = millis();
          }
          vTaskDelay(pdMS_TO_TICKS(100));
        }

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

        nowTX = true;
        transmissionState = phyLayer->startTransmit(data, len);
        TXStart = millis();
        isDo = true;
      }

    } else {
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }
  return isDo;
}

bool LoRaTask::TXDone() {
  TXEnd = millis();
  nowTX = false;

  if (transmissionState != RADIOLIB_ERR_NONE) {
    Serial.print(F("LoRa TX failed, code "));
    Serial.println(transmissionState);
    return true;
  }

  Serial.println(F("LoRa TX Done."));

  Payload xmit;
  if (xQueueReceive(AX25UI_TXQ, &xmit, 0) != pdTRUE) {
    Serial.println("LoRaTXQueue Delete error.");
  }

  return startReceive();
}