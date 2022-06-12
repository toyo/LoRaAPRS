#include <OneButton.h>

#include "boards.h"

class MyBeaconTask {
  OneButton button;
  bool buttonPushed;

  int BeaconPeriodSec;
  unsigned long LastSent = 0;  // millis()

  APRS &aprs;
  String CallSign;

 public:
  std::list<AX25UI> RXQueue;

  MyBeaconTask(APRS &_aprs, uint16_t timeoutSec = 120)
      : aprs(_aprs), button(BUTTON1), buttonPushed(true), BeaconPeriodSec(timeoutSec) {
    button.attachClick([](void *p) { ((MyBeaconTask *)p)->buttonPushed = true; }, this);
  }

  bool setup(String _callsign, uint16_t timeoutSec = 120) {
    BeaconPeriodSec = timeoutSec;
    CallSign = _callsign;
    return true;
  }

  bool loop() {
    button.tick();

    unsigned long ct = millis();
    bool willSend = (ct - LastSent) > (BeaconPeriodSec * 1000);  // timeout?

    if (buttonPushed) {
      willSend = true;
    }

    if (willSend && aprs.getLatLng().isValid()) {
      LastSent = ct;
      buttonPushed = false;

      AX25UI ui(aprs.Encode(), CallSign, aprs.getToCall());
      RXQueue.push_front(ui);
    }
    return true;
  }
};