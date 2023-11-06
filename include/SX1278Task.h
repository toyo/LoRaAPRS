#ifndef LoRaTask_h
#define LoRaTask_h

#include <Arduino.h>

#include "LoRaTask.h"

class SX1278Task : public LoRaTask {
  typedef LoRaTask super;

  SX1278* sx;
  float freq;

  uint8_t syncWord;
  int8_t power;
  uint16_t preambleLength;
  uint8_t gain;

  static volatile SemaphoreHandle_t loraSemaphore;

 protected:
  virtual bool carrierDetected() const { return (sx->getModemStatus() & 0x01) != 0x00; };
  virtual bool signalSynchronized() const { return (sx->getModemStatus() & 0x02) != 0x00; };
  virtual bool validHeaderDetected() const { return (sx->getModemStatus() & 0x08) != 0x00; };
  virtual bool startReceive() const;

 public:
  SX1278Task(bool enableRX = true, bool enableTX = true, uint8_t _syncWord = RADIOLIB_SX127X_SYNC_WORD)
      : LoRaTask(enableRX, enableTX, 1000),  // TXDelay milli sec.
        syncWord(_syncWord){};

  bool setup(float freq, float bw = 20.8, uint8_t sf = 11, uint8_t cr = 6, int8_t power = 2, uint8_t gain = 0,
             uint8_t _dutyPercent = 50, uint16_t preambleLength = 8);

  int16_t setCodingRate(uint8_t cr) { return sx->setCodingRate(cr); }
  int16_t setOutputPower(int8_t power, bool useRfo = false) { return sx->setOutputPower(power, useRfo); }
  int16_t setGain(uint8_t gain) { return sx->setGain(gain); }
  float getRSSI() { return sx->getRSSI(); }
  float getSNR() { return sx->getSNR(); }

  bool loop();
};
#endif