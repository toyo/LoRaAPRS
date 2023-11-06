#include "GPSTask.h"

#ifdef ENABLE_GPS

bool GPSTask::setup() {
  LatLng::setup();
  Serial1.begin(9600, SERIAL_8N1, rxPin, txPin);
  return true;
}

bool GPSTask::loop() {
  LatLng::loop();
  while (Serial1.available() > 0) {
    encode(Serial1.read());
  }
  return true;
}

bool GPSTask::isValid() const { return location.isValid(); }

int32_t GPSTask::getLat1e7() {
  if (location.isValid()) {
    return location.lat() * 1e7;
  } else {
    return LatLng::getLat1e7();
  }
}

int32_t GPSTask::getLng1e7() {
  if (location.isValid()) {
    return location.lng() * 1e7;
  } else {
    return LatLng::getLng1e7();
  }
}
#endif