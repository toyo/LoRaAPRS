#include "MyBeaconTask.h"

volatile SemaphoreHandle_t MyBeaconTask::beaconSemaphore;

void IRAM_ATTR MyBeaconTask::onBeacon() { xSemaphoreGiveFromISR(MyBeaconTask::beaconSemaphore, NULL); }

bool MyBeaconTask::setup(String _callsign, uint timeoutSec) {
  beaconSemaphore = xSemaphoreCreateBinary();
  xAutoReloadTimer = xTimerCreate("AutoReload", pdMS_TO_TICKS(timeoutSec * 1000), pdTRUE, 0,
                                  [](void*) { xSemaphoreGive(MyBeaconTask::beaconSemaphore); });
  xTimerStart(xAutoReloadTimer, portMAX_DELAY);

#if BUTTON1 != -1
  attachInterrupt(BUTTON1, onBeacon, RISING);
#endif

  CallSign = _callsign;
  return true;
}

bool MyBeaconTask::loop() {
  task(0);
  return true;
}

void MyBeaconTask::task(portTickType xBlockTime) {
  if (xSemaphoreTake(beaconSemaphore, xBlockTime) == pdTRUE) {
    Serial.println("Send Beacon");
    if (aprs.getLatLng().isValid()) {
      AX25UI ui(aprs.Encode(), CallSign.c_str(), aprs.getToCall().c_str());
      xQueueSendToFront(TXQueue, &ui, portMAX_DELAY);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
    while (xSemaphoreTake(beaconSemaphore, 0) == pdTRUE)
      ;  // to avoid chattering
  }
}
