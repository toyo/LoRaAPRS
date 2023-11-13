#ifndef WiFiTask_h
#define WiFiTask_h

#include "boards.h"

#ifdef ENABLE_WIFI
#include <WiFi.h>
#endif  // ENABLE_WIFI

#include <list>

#include "LatLng.h"
#include "Payload.h"

class WiFiTask {
#ifdef ENABLE_WIFI
  WiFiClient client;
  wl_status_t laststatus = WL_NO_SHIELD;

  char callsign[16];
  char passcode[8];
  String filter;
  String host;
  int httpPort;

  bool enableRX;
  bool enableTX;

  uint8_t recvline[256];
  uint8_t *endptr = recvline;

  const char *wl_status_t_String(wl_status_t x) {
    switch (x) {
      case WL_NO_SHIELD:
        return "WL_NO_SHIELD";
      case WL_IDLE_STATUS:
        return "WL_IDLE_STATUS";
      case WL_NO_SSID_AVAIL:
        return "WL_NO_SSID_AVAIL";
      case WL_SCAN_COMPLETED:
        return "WL_SCAN_COMPLETED";
      case WL_CONNECTED:
        return "WL_CONNECTED";
      case WL_CONNECT_FAILED:
        return "WL_CONNECT_FAILED";
      case WL_CONNECTION_LOST:
        return "WL_CONNECTION_LOST";
      case WL_DISCONNECTED:
        return "WL_DISCONNECTED";
    }
    return "Unknown Wifi Status.";
  }
#endif  // ENABLE_WIFI

  QueueHandle_t &WifiRXQ;

 public:
  WiFiTask(QueueHandle_t &_RXQ) : WifiRXQ(_RXQ){};

  bool setup(LatLng *l, uint distKm = 50, const char *callsign = "N0CALL", const char *passcode = "00000",
             const char *host = "rotate.aprs2.net", const int httpPort = 14580, bool enableRX = true,
             bool enableTX = true);
  bool loop(const char *SSID, const char *password);

  std::list<Payload> TXQueue;
};
#endif
