#include <RadioLib.h>
#include <Wire.h>

#include <cmath>
#include <map>

#include "APRS.h"
#include "AX25UI.h"
#include "AX25UITask.h"
#include "GPSTask.h"
#include "MyBeaconTask.h"
#include "OLEDTask.h"
#include "PMUTask.h"
#include "SX1278Task.h"
#include "WiFiTask.h"
#include "config.h"

Config c;

#if defined(XPOWERS_CHIP_AXP192)
PMUTask PMU;
#endif
WiFiTask Wifi;
SX1278Task LoRa;
GPSTask GPS;

AX25UITask WiFiAX25(Wifi.RXQueue, Wifi.TXQueue);
AX25UITask LoRaAX25(LoRa.AX25UI_RXQueue, LoRa.AX25UI_TXQueue);

APRS aprsHere(GPS);
MyBeaconTask MyBeacon(aprsHere);
OLEDTask OLED(GPS);

void setup() {
  Serial.begin(115200);

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Wakeup caused by external signal using RTC_IO");
      break;
    default:
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
      break;
  }

  c.begin();

#if defined(XPOWERS_CHIP_AXP192)
  PMU.setup();
#endif
  OLED.setup();

  LoRa.setup(c.channel.frequency * (c.lora.offsetppm * 1e-6 + 1), c.channel.bandwidth, c.channel.spreading_factor,
             c.lora.coding_rate4, c.lora.power, c.lora.gain_rx, 30, 8);
  LoRaAX25.setup();
  LoRaAX25.addUITRACE(c.digi.uitrace);
  LoRaAX25.setCallSign(c.callsign, true);

  GPS.setup();
  aprsHere.set(c.beacon.latitude, c.beacon.longitude, c.beacon.ambiguity, c.beacon.message);

  if (c.beacon.timeoutSec != 0) {
    Wifi.setup(nullptr, 100, c.callsign.c_str(), c.passcode.c_str(), c.aprs_is.server.c_str());
  } else {
    Wifi.setup(&aprsHere.getLatLng(), 100, c.callsign.c_str(), c.passcode.c_str(), c.aprs_is.server.c_str());
  }

  WiFiAX25.setup();
  WiFiAX25.setCallSign(c.callsign, false);

  MyBeacon.setup(c.callsign, c.beacon.timeoutSec);

  return;
}

void loop() {
#if defined(XPOWERS_CHIP_AXP192)
  PMU.loop();
#endif
  OLED.loop();
  GPS.loop();

  MyBeacon.loop();

  while (!MyBeacon.RXQueue.empty()) {
    AX25UI ui = MyBeacon.RXQueue.front();
    WiFiAX25.TXQueue.push_front(ui);
    if (c.beacon.digipath != "") {
      ui.AppendDigiCall(c.beacon.digipath);
    }
    LoRaAX25.TXQueue.push_front(ui);
    MyBeacon.RXQueue.pop_front();
  }

  Wifi.loop(c.wifi.SSID.c_str(), c.wifi.password.c_str());
  WiFiAX25.loop();

  while (!WiFiAX25.RXQueue.empty()) {
    static std::map<String, uint32_t> distList;
    static uint32_t distanceCentiMeter = 3000000;

    AX25UI ui = WiFiAX25.RXQueue.front();
    APRS aprsThere(ui);
    if (aprsThere.HasLocation()) {
      distList[ui.getFromCall()] = aprsHere.distancecmFrom(aprsThere);
    }
    if (distList.find(ui.getFromCall()) != distList.end()) {
      if (distList[ui.getFromCall()] < distanceCentiMeter) {
        size_t txQueueSize = LoRaAX25.TXQueueSize();
        if (txQueueSize >= 1) {
          distanceCentiMeter /= pow(1.05, txQueueSize);
          Serial.println(String(F("Distance decrease: ")) + distanceCentiMeter / 100 + F("m, more ") + txQueueSize +
                         F(" packets"));
          if (!aprsThere.HasLocation()) {
            goto done;
          }
        }
        ui.EraseDigiCalls();
        ui.AppendDigiCall("TCPIP");  // http://www.aprs-is.net/IGateDetails.aspx
        ui.AppendDigiCall(c.callsign + "*");
        LoRaAX25.TXQueue.push_back(ui);
      } else {
        if (LoRaAX25.TXQueueSize() == 0) {
          distanceCentiMeter *= 1.05;
          Serial.println(String(F("Distance increase: ")) + distanceCentiMeter / 100 + F("m."));
        }
      }
    }

  done:
    OLED.ShowUI(ui);
    WiFiAX25.RXQueue.pop_front();
  }

  LoRa.loop();
  LoRaAX25.loop();

  while (!LoRaAX25.RXQueue.empty()) {
    AX25UI ui = LoRaAX25.RXQueue.front();
    if (ui.isIGATEable()) {
      ui.AppendDigiCall(c.callsign);
      ui.AppendDigiCall("I");  // http://www.aprs-is.net/q.aspx , to set qAR.
      WiFiAX25.TXQueue.push_back(ui);
    }
    OLED.ShowUI(ui);
    LoRaAX25.RXQueue.pop_front();
  }
}
