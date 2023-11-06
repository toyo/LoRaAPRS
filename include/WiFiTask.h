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
#endif  // ENABLE_WIFI

 public:
  WiFiTask(){};

  bool setup(LatLng *l, uint distKm = 50, const char *callsign = "N0CALL", const char *passcode = "00000",
             const char *host = "rotate.aprs2.net", const int httpPort = 14580, bool enableRX = true,
             bool enableTX = true);
  bool loop(const char *SSID, const char *password);

  std::list<Payload> RXQueue;
  std::list<Payload> TXQueue;
};