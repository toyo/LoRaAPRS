#ifndef LoRaTask_h
#define LoRaTask_h
#include <RadioLib.h>

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
  bool nowTX;

  unsigned long RXEnd = 0, RXCarrierDetected = 0;

  int transmissionState = RADIOLIB_ERR_NONE;

  virtual bool carrierDetected() const = 0;
  virtual bool validHeaderDetected() const = 0;
  virtual bool startReceive() const = 0;

 protected:
  float bw;
  uint8_t sf;
  uint8_t cr;

  QueueHandle_t& AX25UI_RXQ;
  QueueHandle_t& AX25UI_TXQ;

 public:
  LoRaTask() = delete;
  LoRaTask(QueueHandle_t& _RXQ, QueueHandle_t& _TXQ, bool enableRX = true, bool enableTX = true,
           const uint txDelay = 1000, const bool addCR = false)
      : AX25UI_RXQ(_RXQ), AX25UI_TXQ(_TXQ), enableRX(enableRX), enableTX(enableTX), TXDelay(txDelay), AddCR(addCR){};

  bool setup(PhysicalLayer* pl);

  bool taskTX(portTickType xBlockTime);

  bool TXDone();
};
#endif