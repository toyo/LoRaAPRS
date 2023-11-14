#ifndef MyBeaconTask_h
#define MyBeaconTask_h

#include <Arduino.h>

#include "APRS.h"
#include "boards.h"

class MyBeaconTask {
  APRS &aprs;
  String CallSign;

  static volatile SemaphoreHandle_t beaconSemaphore;
  static void onBeacon();

  QueueHandle_t &TXQueue;

  TimerHandle_t xAutoReloadTimer;

 public:
  MyBeaconTask(APRS &_aprs, QueueHandle_t &_TXQueue) : aprs(_aprs), TXQueue(_TXQueue) {}

  bool setup(String _callsign, uint timeoutSec);

  bool loop();
  void task(portTickType xBlockTime);
};
#endif