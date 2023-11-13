#include <RadioLib.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>

#include <cmath>
#include <map>

#include "APRS.h"
#include "AX25UI.h"
#include "AX25UITask.h"
#include "GPSTask.h"
#include "MyBeaconTask.h"
#include "OLEDTask.h"
#include "SX1278Task.h"
#include "WiFiTask.h"
#include "config.h"

Config c;

#if defined(XPOWERS_CHIP_AXP192)
#include "AXP192Task.h"
#endif

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

#if defined(XPOWERS_CHIP_AXP192)
  xTaskCreateUniversal(
      [](void *) {
        static AXP192Task AXP192;
        AXP192.setup();
        while (1) {
          bool isDo = false;
          isDo = AXP192.loop() || isDo;
          if (!isDo) {
            vTaskDelay(500 / portTICK_RATE_MS);
          }
        }
      },
      "AXP192Task", 4096, NULL, 1, NULL, CONFIG_ARDUINO_RUNNING_CORE);
#endif

  c.begin();

  static GPSTask GPS;

  xTaskCreateUniversal(
      [](void *) {
        GPS.setup();
        while (1) {
          if (!GPS.loop()) delay(100);
        }
      },
      "GPSTask", 4096, NULL, 1, NULL, CONFIG_ARDUINO_RUNNING_CORE);

  static APRS aprsHere(GPS);
  aprsHere.set(c.beacon.latitude, c.beacon.longitude, c.beacon.ambiguity, c.beacon.message);
  static MyBeaconTask MyBeacon(aprsHere);

  static OLEDTask OLED(GPS);
  xTaskCreateUniversal(
      [](void *) {
        OLED.setup();
        while (1) {
          if (!OLED.loop()) delay(100);
        }
      },
      "OLEDTask", 4096, NULL, 1, NULL, CONFIG_ARDUINO_RUNNING_CORE);

  //

  // LoRa

  static QueueHandle_t LoRaToAX25Q = xQueueCreate(4, sizeof(LoRaRXPayload));
#if defined(ARDUINO_TTGO_LoRa32_v21new)
  static SX1278Task LoRa(LoRaToAX25Q, LORA_CS, LORA_IRQ, RADIOLIB_NC, RADIOLIB_NC);
#elif defined(ARDUINO_T_Beam)
  static SX1278Task LoRa(LoRaToAX25Q, LORA_CS, LORA_IRQ, LORA_RST, LORA_IO1);
#elif defined(ARDUINO_NRF52840_PCA10056)
  static SX1278Task LoRa(LoRaToAX25Q, RADIOLIB_NC, RADIOLIB_NC, RADIOLIB_NC, RADIOLIB_NC);
#else
#error Unknown board.
#endif

  SPI.begin(SCK, MISO, MOSI);
  LoRa.setup(SPI, c.channel.frequency * (c.lora.offsetppm * 1e-6 + 1), c.channel.bandwidth, c.channel.spreading_factor,
             c.lora.coding_rate4, c.lora.power, c.lora.gain_rx, 30, 8);

  xTaskCreateUniversal(  // LoRaRX
      [](void *) {
        while (1) {
          LoRa.taskRX(portMAX_DELAY);
        }
      },
      "LoraRXTask", 4096, NULL, 1, NULL, CONFIG_ARDUINO_RUNNING_CORE);

  xTaskCreateUniversal(  // LoRaTX
      [](void *) {
        while (1) {
          if (!LoRa.taskTX()) {
            vTaskDelay(500 / portTICK_RATE_MS);
          }
        }
      },
      "LoRaTXTask", 4096, NULL, 1, NULL, CONFIG_ARDUINO_RUNNING_CORE);

  static QueueHandle_t LoRaAX25toUserQ = xQueueCreate(4, sizeof(AX25UI));
  static AX25UITask LoRaAX25(LoRaToAX25Q, LoRa.AX25UI_TXQueue, LoRaAX25toUserQ);

  xTaskCreateUniversal(  // LoRa AX25
      [](void *) {
        LoRaAX25.setup();
        LoRaAX25.addUITRACE(c.digi.uitrace);
        LoRaAX25.setCallSign(c.callsign, true);
        while (1) {
          if (!LoRaAX25.loop()) delay(100);
        }
      },
      "LoRaAX25Task", 4096, NULL, 1, NULL, CONFIG_ARDUINO_RUNNING_CORE);

  // WiFi
#ifdef ENABLE_WIFI

  static QueueHandle_t WiFiToAX25Q = xQueueCreate(4, sizeof(LoRaRXPayload));
  static WiFiTask Wifi(WiFiToAX25Q);

  xTaskCreateUniversal(  // WiFi
      [](void *) {
        if (c.beacon.timeoutSec != 0) {
          Wifi.setup(nullptr, 100, c.callsign.c_str(), c.passcode.c_str(), c.aprs_is.server.c_str());
        } else {
          Wifi.setup(&aprsHere.getLatLng(), 100, c.callsign.c_str(), c.passcode.c_str(), c.aprs_is.server.c_str());
        }
        while (1) {
          if (!Wifi.loop(c.wifi.SSID.c_str(), c.wifi.password.c_str())) delay(100);
        }
      },
      "WiFiTask", 4096, NULL, 1, NULL, CONFIG_ARDUINO_RUNNING_CORE);

  static QueueHandle_t WiFiAX25toUserQ = xQueueCreate(10, sizeof(AX25UI));
  static AX25UITask WiFiAX25(WiFiToAX25Q, Wifi.TXQueue, WiFiAX25toUserQ);

  xTaskCreateUniversal(  // WiFiAX25
      [](void *) {
        WiFiAX25.setup();
        WiFiAX25.setCallSign(c.callsign, false);
        while (1) {
          if (!WiFiAX25.loop()) delay(100);
        }
      },
      "WiFiAX25Task", 4096, NULL, 1, NULL, CONFIG_ARDUINO_RUNNING_CORE);

#endif

  // Beacon
  MyBeacon.setup(c.callsign, c.beacon.timeoutSec);

  xTaskCreateUniversal(  // Beacon
      [](void *) {
        while (1) {
          MyBeacon.task(portMAX_DELAY);
        }
      },
      "MyBeaconTask", 4096, NULL, 1, NULL, CONFIG_ARDUINO_RUNNING_CORE);

  //
  // init done.
  //

  xTaskCreateUniversal(  // From LoRa
      [](void *) {
        while (1) {
          AX25UI ui;
          if (xQueueReceive(LoRaAX25toUserQ, &ui, portMAX_DELAY) == pdTRUE) {
            OLED.ShowUI(ui);
#ifdef ENABLE_WIFI
            if (ui.isIGATEable()) {
              ui.AppendDigiCall(c.callsign.c_str());
              ui.AppendDigiCall("I");  // http://www.aprs-is.net/q.aspx , to set qAR.
              WiFiAX25.TXQueue.push_back(ui);
            }
#endif
          }
        }
      },
      "LoRaUserTask", 4096, NULL, 1, NULL, CONFIG_ARDUINO_RUNNING_CORE);

#ifdef ENABLE_WIFI
  xTaskCreateUniversal(  // From WiFi
      [](void *) {
        while (1) {
          AX25UI ui;
          if (xQueueReceive(WiFiAX25toUserQ, &ui, portMAX_DELAY) == pdTRUE) {
            static std::map<String, uint32_t> distList;
            static uint32_t distanceCentiMeter = 3000000;

            APRS aprsThere(ui);
            if (aprsThere.HasLocation()) {
              distList[ui.getFromCall()] = aprsHere.distancecmFrom(aprsThere);
            }
            if (distList.find(ui.getFromCall()) != distList.end()) {
              if (distList[ui.getFromCall()] < distanceCentiMeter) {
                size_t txQueueSize = LoRaAX25.TXQueueSize();
                if (txQueueSize >= 1) {
                  distanceCentiMeter /= pow(1.05, txQueueSize);
                  Serial.println(String(F("Distance decrease: ")) + distanceCentiMeter / 100 + F("m, more ") +
                                 txQueueSize + F(" packets"));
                  if (!aprsThere.HasLocation()) {
                    goto done;
                  }
                }
                ui.EraseDigiCalls();
                ui.AppendDigiCall("TCPIP");  // http://www.aprs-is.net/IGateDetails.aspx
                ui.AppendDigiCall((c.callsign + "*").c_str());
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
          }
        }
      },
      "WiFiUserTask", 4096, NULL, 1, NULL, CONFIG_ARDUINO_RUNNING_CORE);
#endif  //  ENABLE_WIFI

  xTaskCreateUniversal(  // From Beacon
      [](void *) {
        while (1) {
          if (!MyBeacon.TXQueue.empty()) {
            AX25UI ui = MyBeacon.TXQueue.front();
#ifdef ENABLE_WIFI
            WiFiAX25.TXQueue.push_front(ui);
#endif
            if (c.beacon.digipath != "") {
              ui.AppendDigiCall(c.beacon.digipath.c_str());
            }
            LoRaAX25.TXQueue.push_front(ui);
            MyBeacon.TXQueue.pop_front();
          } else {
            vTaskDelay(500 / portTICK_RATE_MS);
          }
        }
      },
      "BeaconGenerateTask", 4096, NULL, 1, NULL, CONFIG_ARDUINO_RUNNING_CORE);
}

void loop() { vTaskSuspend(NULL); }
