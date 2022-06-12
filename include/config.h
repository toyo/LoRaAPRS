#include <WString.h>

class ChannelConfig {
 public:
  float frequency;
  int spreading_factor;
  float bandwidth;
};

class LoRaConfig {
 public:
  int gain_rx;
  int power;
  int coding_rate4;
  bool tx_enable;
  float offsetppm;
};

class WiFiConfig {
 public:
  String SSID;
  String password;
};

class APRSISConfig {
 public:
  String server;
  int port;
};

class BeaconConfig {
 public:
  String message;
  String digipath;
  float latitude;
  float longitude;
  uint8_t ambiguity;
  bool use_gps;
  uint16_t timeoutSec;
};

class DigiConfig {
 public:
  String uitrace;
};

class Config {
 public:
  ChannelConfig channel;

  String callsign;
  String passcode;

  LoRaConfig lora;
  APRSISConfig aprs_is;
  BeaconConfig beacon;
  WiFiConfig wifi;
  DigiConfig digi;

  bool begin();
};
