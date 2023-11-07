#ifndef __AX25UI_H_
#define __AX25UI_H_

#include <WString.h>

#include <deque>

class AX25UI {
  String FromCall;
  std::deque<String> ToDigiCalls;
  String message;

 public:
  AX25UI() = delete;
  AX25UI(const AX25UI& c) : FromCall(c.FromCall), ToDigiCalls(c.ToDigiCalls), message(c.message){};

  AX25UI(const uint8_t* data, const size_t len);                 // From RX Message.
  AX25UI(String msg, String from, String to, String digi = "");  // From TX Message.

  String Encode() const;  // To TX Message.

  bool isNull() const { return FromCall == ""; }
  bool isIGATEable() const {
    return ToDigiCalls.size() > 1 && ToDigiCalls[1] != "TCPIP" && ToDigiCalls[1] != "TCPXX" &&
           ToDigiCalls[1] != "NOGATE" && ToDigiCalls[1] != "RFONLY" &&
           message[0] != '?';  // http://www.aprs-is.net/IGateDetails.aspx
  }

  String getToDigiCalls(int i) const { return ToDigiCalls[i]; };
  bool findDigiCall(String _call) const {
    for (String call : ToDigiCalls) {
      if (call == _call) {
        return true;
      }
    }
    return false;
  }
  int8_t findNextDigiIndex() const {
    int len = ToDigiCalls.size();
    for (int digiindex = 1; digiindex < len; digiindex++) {
      if (ToDigiCalls[digiindex].indexOf("*") == -1) {
        return digiindex;
      }
    }
    return -1;
  }
  String getFromCall() const { return FromCall; }
  String GetToCall() const { return ToDigiCalls[0]; }
  String Payload() const { return message; }
  void setToDigiCall(String call, int num) { ToDigiCalls[num] = call; }
  void addToDigiCall(String call, int num) { ToDigiCalls.insert(ToDigiCalls.begin() + num, call); }
  void EraseDigiCalls() { ToDigiCalls.erase(ToDigiCalls.begin() + 1, ToDigiCalls.end()); }
  void AppendDigiCall(String digicall) { ToDigiCalls.push_back(digicall); }
};
#endif  //__AX25UI_H_