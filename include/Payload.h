#ifndef Payload_h
#define Payload_h
#include <RadioLib.h>
#include <string.h>

class Payload {
 protected:
  uint8_t data[257];  // MAXLORALEN=256 + null for Terminate.
  size_t len;

 public:
  Payload(const uint8_t *datas, size_t length) {
    len = length;
    if (len <= sizeof(data)) {
      memcpy(data, datas, len);
    } else {
      len = 0;
    }
  }

  Payload(String s) : Payload((const unsigned char *)s.c_str(), s.length()) {}

  Payload(const Payload &p, bool AddCRLF = false) {
    memcpy(data, p.data, p.len);
    len = p.len;

    if (AddCRLF && (data[len - 1] != '\r' || data[len - 1] != '\n') && len <= sizeof(data) - 1) {
      data[len] = '\r';
      data[len + 1] = '\n';
      data[len + 2] = '\0';
      len += 2;
    }
  }

  const uint8_t *getData() const { return data; }
  size_t getLen() const { return len; }
  String toString() const {
    char s[257];
    memcpy(s, data, len);
    s[len] = '\0';
    String str = s;
    str.replace("\n", "[LF]");
    str.replace("\r", "[CR]");
    return str;
  }
};

class LoRaRXPayload : public Payload {
  float RSSI;
  float SNR;
  bool CRCErr;
  float FrequencyError;

 public:
  LoRaRXPayload(const uint8_t *datas, size_t length) : Payload(datas, length) {}
  LoRaRXPayload(const uint8_t *datas, size_t length, float rssi, float snr, float fErr)
      : Payload(datas, length), RSSI(rssi), SNR(snr), FrequencyError(fErr) {}
  LoRaRXPayload(Payload &lp, float rssi, float snr, float fErr)
      : Payload(lp), RSSI(rssi), SNR(snr), FrequencyError(fErr) {}

  float getRSSI() { return RSSI; }
  float getSNR() { return SNR; }
  float getFrequencyError() { return FrequencyError; }
  bool getCRCErr() { return CRCErr; }

  void setCRCErr(bool err) { CRCErr = err; }
};
#endif