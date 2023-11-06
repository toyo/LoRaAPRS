#ifndef OLEDTask_h
#define OLEDTask_h

#include "APRS.h"

class OLEDTask {
  LatLng &ll;

  AX25UI ui;
  bool updated_ui;

 public:
  OLEDTask(LatLng &_ll) : ll(_ll), updated_ui(false), ui(nullptr, 0) {}

  bool ShowUI(AX25UI &_ui);

  bool setup();
  bool loop();
};
#endif
