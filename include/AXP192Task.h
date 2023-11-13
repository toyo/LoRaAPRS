#ifndef AXP192Task_h
#define AXP192Task_h

#include <Arduino.h>

#include "boards.h"

#if defined(XPOWERS_CHIP_AXP192)

class AXP192Task {
  volatile SemaphoreHandle_t pmuSemaphore;

 public:
  bool setup();
  void task();
};
#endif

#endif