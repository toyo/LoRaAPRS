#ifndef MyBeaconTask_h
#define MyBeaconTask_h

#include <Arduino.h>

#include <list>

#include "APRS.h"
#include "boards.h"

class MyBeaconTask {
  APRS &aprs;
  String CallSign;

  static volatile SemaphoreHandle_t beaconSemaphore;
  static void onBeacon();

 public:
  std::list<AX25UI> RXQueue;

  MyBeaconTask(APRS &_aprs) : aprs(_aprs) {}

  bool setup(String _callsign, uint timeoutSec);

  bool loop();
  bool task(portTickType xBlockTime);
};
#endif