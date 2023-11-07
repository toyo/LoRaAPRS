#include "MyBeaconTask.h"

volatile SemaphoreHandle_t MyBeaconTask::beaconSemaphore;

hw_timer_t* timer = NULL;

void IRAM_ATTR MyBeaconTask::onBeacon() { xSemaphoreGiveFromISR(MyBeaconTask::beaconSemaphore, NULL); }

bool MyBeaconTask::setup(String _callsign, uint timeoutSec) {
  beaconSemaphore = xSemaphoreCreateBinary();
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, onBeacon, true);
  timerAlarmWrite(timer, timeoutSec * 1000000, true);
  timerAlarmEnable(timer);

#if BUTTON1 != -1
  attachInterrupt(BUTTON1, onBeacon, RISING);
#endif

  CallSign = _callsign;
  return true;
}

bool MyBeaconTask::loop() {
  if (xSemaphoreTake(beaconSemaphore, 0) == pdTRUE) {
    Serial.println("Send Beacon");
    if (aprs.getLatLng().isValid()) {
      AX25UI ui(aprs.Encode(), CallSign, aprs.getToCall());
      RXQueue.push_front(ui);
      return true;
    } else {
      return false;
    }
    while (xSemaphoreTake(beaconSemaphore, 0) == pdTRUE)
      ;  // to avoid chattering
  }
  return false;
}
