#ifndef __AX25UI_H_
#define __AX25UI_H_

#include <WString.h>

#define MAXCALLSIGNLEN 16
#define MAXMSGLEN 300

class AX25UI {
  char FromCall[MAXCALLSIGNLEN];
  char ToCall[MAXCALLSIGNLEN];
  char DigiCalls[8][MAXCALLSIGNLEN];
  uint8_t numDigiCalls;
  char message[MAXMSGLEN];

 public:
  AX25UI(const uint8_t* data = NULL, const size_t len = 0);                       // From RX Message.
  AX25UI(String msg, const char* from, const char* to, const char* digi = NULL);  // From TX Message.

  String Encode() const;  // To TX Message.

  bool isNull() const { return FromCall[0] == 0; }
  bool isIGATEable() const;

  String getDigiCalls(int i) const { return DigiCalls[i]; };

  bool findDigiCall(const char* _call) const {
    for (int i = 0; i < numDigiCalls; i++) {
      if (strncmp(DigiCalls[i], _call, MAXCALLSIGNLEN) == 0) {
        return true;
      }
    }
    return false;
  }
  int8_t findNextDigiIndex() const {
    int len = numDigiCalls;
    for (int digiindex = 0; digiindex < len; digiindex++) {
      if (String(DigiCalls[digiindex]).indexOf("*") == -1) {
        return digiindex;
      }
    }
    return -1;
  }
  const char* getFromCall() { return FromCall; }
  String GetToCall() const { return ToCall; }
  String Payload() const { return message; }
  void setToDigiCall(const char* call, int num) {
    strlcpy(DigiCalls[num], call, MAXCALLSIGNLEN);
    if (numDigiCalls < num) {
      numDigiCalls = num;
    }
  }
  void addToDigiCall(const char* call, int num) {
    for (int i = num; i < numDigiCalls + 1; i++) {
      strlcpy(DigiCalls[i + 1], DigiCalls[i], MAXCALLSIGNLEN);
    }
    strlcpy(DigiCalls[num], call, MAXCALLSIGNLEN);
    numDigiCalls++;
  }
  void EraseDigiCalls() { numDigiCalls = 0; }
  void AppendDigiCall(const char* digicall) { strlcpy(DigiCalls[numDigiCalls++], digicall, MAXCALLSIGNLEN); }
};
#endif  //__AX25UI_H_