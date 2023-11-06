#ifndef LoRaTask_h
#define LoRaTask_h
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

  bool setup(PhysicalLayer* pl);

  bool TXloop();

  bool TXDone();
};
#endif