#include "APRS.h"
#include "boards.h"

#ifdef ENABLE_U8G2
#include <U8G2lib.h>
U8G2_SSD1306_128X64_VCOMH0_F_HW_I2C u8g2(U8G2_R0);
#endif

class OLEDTask {
  LatLng &ll;

  AX25UI ui;
  bool updated_ui;

 public:
  OLEDTask(LatLng &_ll) : ll(_ll), updated_ui(false), ui(nullptr, 0) {}

  bool ShowUI(AX25UI &_ui) {
    ui = _ui;
    updated_ui = true;

    return true;
  }

  bool setup() {
#ifdef ENABLE_U8G2
    u8g2.begin();
#endif
    return true;
  }

  bool loop() {
    if (updated_ui) {
      APRS aprsThere(ui);

#ifdef ENABLE_U8G2
      u8g2.clearBuffer();

      u8g2.setFont(u8g2_font_helvR14_tr);
      u8g2.drawStr(0, 16, ui.getFromCall().c_str());

      u8g2.setFont(u8g2_font_helvB10_tf);
      u8g2.drawStr(0, 60, aprsThere.GetMessage().c_str());
#endif
      if (aprsThere.HasLocation()) {
        char far[64];
        far[0] = '\0';

        uint32_t distance = ll.distancecmFrom(aprsThere.getLatLng());
        if (distance >= 100000) {
          strcat(far, ((String)(distance / 100000) + "km").c_str());
        } else {
          strcat(far, ((String)(distance / 100) + "m").c_str());
        }
        strcat(far, "/");

        uint16_t bearing = ll.bearingFrom(aprsThere.getLatLng());
        strcat(far, String(bearing).c_str());
        strcat(far, "\xb0");  // degree

        if (aprsThere.getAltitudeMeter() != 0) {
          strcat(far, "/");
          strcat(far, String(aprsThere.getAltitudeMeter()).c_str());
          strcat(far, "m");
        }

        Serial.print(ui.getFromCall().c_str());
        Serial.print("@");
        Serial.print(aprsThere.Symbol());
        Serial.print("/");
        Serial.print(far);
        Serial.print("/");
        uint16_t course = aprsThere.getCourseDeg();
        if (course != 0) {
          Serial.print(course);
          Serial.print("deg/");
          uint16_t knot = aprsThere.getSpeedKnot();
          Serial.print(uint16_t(float(knot) * 1.852));
          Serial.print("kph/");
        }
        Serial.print(aprsThere.MicEMsg());
        Serial.print("/");

#ifdef ENABLE_U8G2
        u8g2.drawStr(0, 45, far);
#endif

        Serial.println(aprsThere.GetMessage());
      }
#ifdef ENABLE_U8G2
      u8g2.sendBuffer();
#endif
    }
    updated_ui = false;
    return true;
  }
};