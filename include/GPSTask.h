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
  virtual bool setup();
  virtual bool loop();

  virtual bool isValid() const;
  virtual int32_t getLat1e7();
  virtual int32_t getLng1e7();
};
#else   // ENABLE_GPS
typedef LatLng GPSTask;
#endif  // ENABLE_GPS

#endif  // __GPSTASK_H_