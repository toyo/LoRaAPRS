#ifndef APRS_h
#define APRS_h

#include <time.h>

#include "AX25UI.h"
#include "LatLng.h"

enum class MicEMsg : unsigned char {
  UNKNOWN,
  M0,
  M1,
  M2,
  M3,
  M4,
  M5,
  M6,
  C0,
  C1,
  C2,
  C3,
  C4,
  C5,
  C6,
  EMERGENCY,
};

class APRS {  // see http://www.aprs.org/doc/APRS101.PDF
  char DataType;
  char SymbolTableIdentifier;
  char SymbolCode;

  char ObjectName[10];

  float altitudeMeter;

  int8_t ambiguity;

  char message[256];

  struct tm dateTime;

  enum MicEMsg micEMessageType;
  uint16_t courseDeg;
  uint16_t speedKnot;

  bool Decode(const char *ss, const char *toCall);

  LatLng &ll;

 public:
  LatLng &getLatLng() { return ll; }

  uint16_t getCourseDeg() const { return courseDeg; }
  uint16_t getSpeedKnot() const { return speedKnot; }

  // Decode from String.
  APRS(const char *ss, const char *ToCall) : ll(*new LatLng) { Decode(ss, ToCall); };
  APRS(AX25UI ui) : ll(*new LatLng) { Decode(ui.Payload(), ui.GetToCall()); };

  // Set lat/lng manually.
  APRS(LatLng &_ll)
      : ll(_ll),
        DataType('!'),               // pp.17, pp.23.  Position without timestamp
        SymbolTableIdentifier('L'),  // pp.91, pp.104.
        SymbolCode('&')              // pp.91, pp.104.
  {}

  // Set ambiguity/msg manually.
  void set(double _lat, double _lng, int _ambiguity = 2, String _msg = "") {
    ll.set(_lat, _lng);
    ambiguity = _ambiguity;
    memcpy(message, _msg.c_str(), _msg.length());
    message[_msg.length()] = '\0';
  }

  void setDataType(const char c) { DataType = c; }
  void setSymbolTableIdentifier(const char c) { SymbolTableIdentifier = c; }
  void setSymbolCode(const char c) { SymbolCode = c; }

  // Encode to String.
  String Encode() const;

  String getToCall() const { return "APXXX"; }

  bool HasLocation() const {
    return DataType == '/' || DataType == '@' || DataType == '!' || DataType == '=' || DataType == '`' ||
           DataType == '\'';
  }

  void setLatLngAlt(double _lat, double _lng, double _altMeter) {
    ll.setLatDeg1e7(_lat * 1e7);
    ll.setLngDeg1e7(_lng * 1e7);
    altitudeMeter = _altMeter;
  }

  uint32_t distancecmFrom(APRS &_aprs) { return ll.distancecmFrom(_aprs.ll); }

  int32_t getAltitudeMeter() const { return altitudeMeter; }

  String GetMessage() const { return message; }

  String Symbol() const;
  String MicEMsg() const {
    switch (micEMessageType) {
      case MicEMsg::EMERGENCY:
        return "Emergency";
      case MicEMsg::M0:
        return "M0: Off Duty";
      case MicEMsg::M1:
        return "M1: En Route";
      case MicEMsg::M2:
        return "M2: In Service";
      case MicEMsg::M3:
        return "M3: Returning";
      case MicEMsg::M4:
        return "M4: Committed";
      case MicEMsg::M5:
        return "M5: Special";
      case MicEMsg::M6:
        return "M6: Priority";
      case MicEMsg::C0:
        return "C0: Custom-0";
      case MicEMsg::C1:
        return "C1: Custom-1";
      case MicEMsg::C2:
        return "C2: Custom-2";
      case MicEMsg::C3:
        return "C3: Custom-3";
      case MicEMsg::C4:
        return "C4: Custom-4";
      case MicEMsg::C5:
        return "C5: Custom-5";
      case MicEMsg::C6:
        return "C6: Custom-6";
      default:
        return "";
    }
  }
};

#endif