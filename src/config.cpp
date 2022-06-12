#include "config.h"

#include <ArduinoJson.h>
#include <SPIFFS.h>

bool Config::begin() {
  String mac_addr;
#ifdef ESP32
  uint8_t mac[8];
  if (esp_efuse_mac_get_default(mac) == ESP_OK) {  // factory-programmed by Espressif in EFUSE.
    char mac_addr_chars[32];

    sprintf(mac_addr_chars, "%02X-%02X-%02X-%02X-%02X-%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    Serial.printf("Factory-programmed Mac Address = %s\r\n", mac_addr_chars);
    mac_addr = mac_addr_chars;
  }
#endif

  bool res = SPIFFS.begin();
  if (!res) {
    return false;
  } else {
#if defined(ARDUINO_TTGO_LoRa32_v21new)
    String configFile = "ttgo-lora32-v21";
#elif defined(ARDUINO_T_Beam)
    String configFile = "ttgo-t-beam";
#endif

    if (SPIFFS.exists("/" + mac_addr + ".json")) {
      configFile = mac_addr;
    } else {
      if (!SPIFFS.exists("/" + configFile + ".json")) {
        SPIFFS.end();
        return false;
      }
    }

    File fd = SPIFFS.open("/" + configFile + ".json", "r");

    Serial.println("/" + configFile + ".json exist");
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, fd);
    fd.close();

    Serial.println("data in file");

    callsign = (const char *)doc["callsign"];
    if (!doc["passcode"].isNull()) {
      passcode = (const char *)doc["passcode"];
    } else {
      passcode = "";
    }

    if (!doc["lora"].isNull()) {
      lora.gain_rx = doc["lora"]["gain_rx"];
      lora.power = doc["lora"]["power"];
      lora.coding_rate4 = doc["lora"]["coding_rate4"];
      lora.tx_enable = doc["lora"]["tx_enable"];
      lora.offsetppm = doc["lora"]["offsetppm"];
    }

    if (!doc["digi"].isNull()) {
      digi.uitrace = (const char *)doc["digi"]["uitrace"];
    }

    if (!doc["aprs_is"].isNull()) {
      aprs_is.server = (const char *)doc["aprs_is"]["server"];
      aprs_is.port = doc["aprs_is"]["port"];
    }

    if (!doc["wifi"].isNull()) {
      wifi.SSID = (const char *)doc["wifi"]["SSID"];
      wifi.password = (const char *)doc["wifi"]["password"];
    }

    if (!doc["beacon"].isNull()) {
      beacon.message = (const char *)doc["beacon"]["message"];
      beacon.digipath = (const char *)doc["beacon"]["digipath"];
      beacon.latitude = doc["beacon"]["latitude"];
      beacon.longitude = doc["beacon"]["longitude"];
      beacon.ambiguity = doc["beacon"]["ambiguity"];
      beacon.use_gps = doc["beacon"]["use_gps"];
      beacon.timeoutSec = doc["beacon"]["timeoutSec"];
    }

    if (SPIFFS.exists("/channel.json")) {
      File fd = SPIFFS.open("/channel.json", "r");
      DynamicJsonDocument doc(256);
      deserializeJson(doc, fd);

      channel.frequency = doc["frequency"];
      channel.spreading_factor = doc["spreading_factor"];
      channel.bandwidth = doc["bandwidth"];

      fd.close();
    } else if (!doc["channel"].isNull()) {
      channel.frequency = doc["channel"]["frequency"];
      channel.spreading_factor = doc["channel"]["spreading_factor"];
      channel.bandwidth = doc["channel"]["bandwidth"];
    } else {
      Serial.println("/channel.json not exist");
    }

    SPIFFS.end();
    return true;
  }
}