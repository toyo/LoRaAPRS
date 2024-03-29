#include "WiFiTask.h"

bool WiFiTask::setup(LatLng *l, uint distKm, const char *callsign, const char *passcode, const char *host,
                     const int httpPort, bool enableRX, bool enableTX) {
#ifdef ENABLE_WIFI
  strncpy(this->callsign, callsign, sizeof(this->callsign));
  if (passcode != nullptr) {
    strncpy(this->passcode, passcode, sizeof(this->passcode));
  } else {
    this->passcode[0] = '\0';
  }
  if (l != nullptr) {
    this->filter = String("r/") + l->getLat() + "/" + l->getLng() + "/" + distKm;
  } else {
    this->filter = String("m/") + distKm;
  }
  // this->filter += " -t/oistunw";
  this->filter += " q//I";
  this->host = host;
  this->httpPort = httpPort;
  this->enableRX = enableRX;
  this->enableTX = enableTX;
#endif

#ifdef ESP32
  uint8_t mac_wifi[6];
  if (esp_read_mac(mac_wifi, ESP_MAC_WIFI_STA) == ESP_OK) {
    Serial.printf("WiFi Mac Address = %02X-%02X-%02X-%02X-%02X-%02X\r\n", mac_wifi[0], mac_wifi[1], mac_wifi[2],
                  mac_wifi[3], mac_wifi[4], mac_wifi[5]);
  }
#endif

#ifdef ENABLE_WIFI
  WiFi.mode(WIFI_STA);
#endif  // ENABLE_WIFI
  return true;
}

void WiFiTask::connect(const char *SSID, const char *password) {
#ifdef ENABLE_WIFI
  wl_status_t status = WiFi.status();

  if (laststatus != status) {
    Serial.print(F("Wifi Status Change :"));
    Serial.print(wl_status_t_String(laststatus));
    Serial.print(F(" to "));
    Serial.println(wl_status_t_String(status));

    if ((laststatus == WL_NO_SHIELD || laststatus == WL_CONNECTED) && status != WL_CONNECTED) {
      Serial.print(F("Connecting to "));
      Serial.println(SSID);
      WiFi.begin(SSID, password);
    } else if (laststatus != WL_CONNECTED && status == WL_CONNECTED) {
      Serial.print(F("WiFi connected. Local IPv4: "));
      Serial.print(WiFi.localIP());
      Serial.print(F(", Local IPv6: "));
      Serial.println(WiFi.localIPv6());
    }
    laststatus = status;
  } else if (status == WL_CONNECTED) {
    Serial.print(F("connecting to "));
    Serial.println(host);

    if (client.connect(host.c_str(), httpPort)) {
      char loginstring[256];
      strcpy(loginstring, "user ");
      strcat(loginstring, callsign);
      strcat(loginstring, " pass ");
      strcat(loginstring, passcode);
      strcat(loginstring, " vers LoRaAPRS 0.00");
      if (filter[0] != '\0') {
        strcat(loginstring, " filter ");
        strcat(loginstring, filter.c_str());
      }
      strcat(loginstring, "\r\n");

      client.println(loginstring);
      Serial.println(loginstring);
    } else {
      Serial.println(F("connection failed."));
      // return false;
    }
  } else {
    vTaskDelay(pdMS_TO_TICKS(500));
  }
#endif  // ENABLE_WIFI
}

void WiFiTask::taskRX(portTickType xBlockTime) {
#ifdef ENABLE_WIFI
  int c;
  while ((c = client.read()) >= 0) {
    *endptr = (uint8_t)c;
    if (*endptr == '\r' || *endptr == '\n') {  // APRS server send [CR][LF].
      if (recvline == endptr) {
        continue;
      } else {
        if (recvline[0] != '#') {
          Payload pkt(recvline, endptr - recvline);
          if (xQueueSend(WifiRXQ, &pkt, xBlockTime) != pdPASS) {
            Serial.println("WiFi RX Buffer Overflow, continues.");
            Serial.print("NO ");
          }
          Serial.print("APRSIS->: ");
          Serial.println(pkt.toString());
        } else {
          *endptr = '\0';
          Serial.println((char *)recvline);
        }
        endptr = recvline;
        break;
      }
    }
    endptr++;
  }
#endif  // ENABLE_WIFI
}

void WiFiTask::taskTX(portTickType xBlockTime) {
  Payload xmit1((const unsigned char *)"", 0);
  while (xQueueReceive(WifiTXQ, &xmit1, xBlockTime) == pdTRUE) {
#ifdef ENABLE_WIFI
    Payload xmit(xmit1, true);
    if (enableTX) {
      Serial.print(F("APRSIS<-: "));
      Serial.println(xmit.toString());
      client.write(xmit.getData(), xmit.getLen());
    }
#endif  // ENABLE_WIFI
  }
}
