#include <Arduino.h>

#include <list>

#include "APRS.h"
#include "boards.h"

extern volatile SemaphoreHandle_t beaconSemaphore;

class MyBeaconTask {
  APRS &aprs;
  String CallSign;

 public:
  std::list<AX25UI> RXQueue;

  MyBeaconTask(APRS &_aprs) : aprs(_aprs) {}

  bool setup(String _callsign, uint timeoutSec);

  bool loop();
};