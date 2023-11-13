#include <AX25UI.h>
#include <Arduino.h>

AX25UI::AX25UI(const uint8_t* data, const size_t len) {
  char buffer[257];
  if (len != 0) {
    memcpy(buffer, data, len);
  }
  buffer[len] = 0;
  ToCall[0] = 0;

  char *saveptr, *retval;

  saveptr = nullptr;
  retval = strtok_r(buffer, ">", &saveptr);
  if (retval == nullptr) {
    return;
  }
  strlcpy(FromCall, retval, MAXCALLSIGNLEN);

  retval = strtok_r(nullptr, ":", &saveptr);
  if (retval == nullptr) {
    return;
  }

  char* p = retval;
  strlcpy(message, saveptr, MAXMSGLEN);

  numDigiCalls = 0;
  saveptr = nullptr;
  while ((retval = strtok_r(p, ",", &saveptr)) != nullptr) {
    if (ToCall[0] == 0) {
      strlcpy(ToCall, retval, MAXCALLSIGNLEN);
    } else {
      strlcpy(DigiCalls[numDigiCalls++], retval, MAXCALLSIGNLEN);
    }
    p = nullptr;
  }
}

AX25UI::AX25UI(String msg, const char* from, const char* to, const char* digi) {
  numDigiCalls = 0;

  strlcpy(FromCall, from, MAXCALLSIGNLEN);
  strlcpy(ToCall, to, MAXCALLSIGNLEN);
  if (digi != NULL) {
    strlcpy(DigiCalls[0], digi, MAXCALLSIGNLEN);
    numDigiCalls = 1;
  }
  strlcpy(message, msg.c_str(), MAXMSGLEN);
}

String AX25UI::Encode() const {
  String s;
  if (FromCall[0] != 0) {
    s = String(FromCall) + ">";
    s += String(ToCall) + ",";

    for (int i = 0; i < numDigiCalls; i++) {
      s += String(DigiCalls[i]) + ",";
    }
    s.remove(s.length() - 1);
    s += ":" + String(message);
  }

  return s;
}

bool AX25UI::isIGATEable() const {
  if (numDigiCalls != 0) {
    return DigiCalls[0] != "TCPIP" && DigiCalls[0] != "TCPXX" && DigiCalls[0] != "NOGATE" && DigiCalls[0] != "RFONLY" &&
           message[0] != '?';  // http://www.aprs-is.net/IGateDetails.aspx
  } else {
    Serial.println("NO DIGI to IGATE");
    return true;
  }
}