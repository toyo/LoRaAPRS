#ifndef __GPSTASK_H_
#define __GPSTASK_H_
#include "LatLng.h"
#include "boards.h"

#ifdef ENABLE_GPS
#include <TinyGPSPlus.h>

class GPSTask : public TinyGPSPlus, public LatLng {
  int8_t rxPin;
  int8_t txPin;

  unsigned int lastMillis;

 public:
  GPSTask(int8_t rxPin = 34 /*RX*/, int8_t txPin = 12 /*TX*/) : rxPin(rxPin), txPin(txPin){};

  virtual bool setup() {
    LatLng::setup();
    Serial1.begin(9600, SERIAL_8N1, rxPin, txPin);
    return true;
  }

  virtual bool loop() {
    LatLng::loop();
    while (Serial1.available() > 0) {
      encode(Serial1.read());
    }
    return true;
  }

  virtual bool isValid() const { return location.isValid(); }

  virtual int32_t getLat1e7() {
    if (location.isValid()) {
      return location.lat() * 1e7;
    } else {
      return LatLng::getLat1e7();
    }
  }

  virtual int32_t getLng1e7() {
    if (location.isValid()) {
      return location.lng() * 1e7;
    } else {
      return LatLng::getLng1e7();
    }
  }
};
#else   // ENABLE_GPS
typedef LatLng GPSTask;
#endif  // ENABLE_GPS

#endif  // __GPSTASK_H_