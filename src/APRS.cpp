#include "APRS.h"

#include "LatLng.h"

uint8_t MicEDestDecodeFlag(char c) {
  if (('0' <= c && c <= '9') || c == 'L') {
    return 0;
  } else if ('P' <= c && c <= 'Z') {
    return 1;
  } else if ('A' <= c && c <= 'K') {
    return 2;
  }
  return 3;
}

uint8_t MicEDestDecodeDigit(char c, uint8_t space) {
  if (c == 'K' || c == 'L' || c == 'Z') {
    return space;
  } else if ('0' <= c && c <= '9') {
    return c - '0';
  } else if ('A' <= c && c <= 'J') {
    return c - 'A';
  } else if ('P' <= c && c <= 'Y') {
    return c - 'P';
  }
  return space;
}

uint8_t MicEHeaderDecodeDegree(uint8_t c, bool longOffset100) {
  c -= 28;
  c = longOffset100 ? c + 100 : c;
  if (180 <= c && c <= 189) {
    c -= 80;
  } else if (190 <= c && c <= 199) {
    c -= 190;
  }
  return c;
}

uint8_t MicEHeaderDecodeMinute(uint8_t c) {
  c -= 28;
  if (60 <= c) {
    c -= 60;
  }
  return c;
}

// Decode from String.
bool APRS::Decode(const char *ss, const char *toCall) {
  char *s_org = strdup(ss);
  char *s = s_org;
  DataType = s[0];
  s++;

  switch (DataType) {
    case ';':  // Object Report Format. pp.58.
      memcpy(ObjectName, s, 9);
      s[9] = '\0';
      s += 10;
    case '/':  // Lat/Long Position Report Format — with Timestamp
    case '@':  // Lat/Long Position Report Format — with Timestamp
    case '*':  // a live Object. pp.58.
      switch (s[6]) {
        case '/':  // DHM localtime
                   // no manage timezone
        case 'z':  // DHM UTC
          dateTime.tm_mday = (s[0] - '0') * 10 + (s[1] - '0');
          dateTime.tm_hour = (s[2] - '0') * 10 + (s[3] - '0');
          dateTime.tm_min = (s[4] - '0') * 10 + (s[5] - '0');
          dateTime.tm_sec = 0;
          break;
        case 'h':  // HMS UTC
          dateTime.tm_mday = 0;
          dateTime.tm_hour = (s[0] - '0') * 10 + (s[1] - '0');
          dateTime.tm_min = (s[2] - '0') * 10 + (s[3] - '0');
          dateTime.tm_sec = (s[4] - '0') * 10 + (s[5] - '0');
      }
      s += 7;
    case '!':  // Lat/Long Position Report Format — without Timestamp & a live Item. pp.59.
    case '=':  // Lat/Long Position Report Format — without Timestamp
      micEMessageType = MicEMsg::UNKNOWN;

      if (isdigit(s[0]) && isdigit(s[1]) && s[4] == '.' && isdigit(s[9]) && isdigit(s[10]) && isdigit(s[11]) &&
          s[14] == '.') {  // Chapter 6: Time and Position Formats
        if (s[2] == ' ') {
          ambiguity = 4;
        } else if (s[3] == ' ') {
          ambiguity = 3;
        } else if (s[5] == ' ') {
          ambiguity = 2;
        } else if (s[6] == ' ') {
          ambiguity = 1;
        }

        switch (ambiguity) {
          case 4:
            s[2] = '0';
            s[12] = '0';
          case 3:
            s[3] = '0';
            s[13] = '0';
          case 2:
            s[5] = '0';
            s[15] = '0';
          case 1:
            s[6] = '0';
            s[16] = '0';
        }

        SymbolTableIdentifier = s[8];

        ll.setLatDegMin1e7((s[0] - '0') * 10 + (s[1] - '0'), s[7] == 'N',
                           uint32_t(s[2] - '0') * 1e8 + uint32_t(s[3] - '0') * 1e7 + uint32_t(s[5] - '0') * 1e6 +
                               uint32_t(s[6] - '0') * 1e5);
        ll.setLngDegMin1e7((s[9] - '0') * 100 + (s[10] - '0') * 10 + (s[11] - '0'), s[17] == 'E',
                           uint32_t(s[12] - '0') * 1e8 + uint32_t(s[13] - '0') * 1e7 + uint32_t(s[15] - '0') * 1e6 +
                               uint32_t(s[16] - '0') * 1e5);

        SymbolCode = s[18];
        s += 19;
      } else {
        SymbolTableIdentifier = s[0];

        // 1-4 Compressed Lat YYYY, 46*91*91=380926, red line is 46*90=4140 / 4140*91*91=45*91*91*91+45*91*91
        ll.setLatDeg1e7(-int64_t(int32_t(s[1] - 33 - 45) * 91 * 91 * 91 + int32_t(s[2] - 33 - 45) * 91 * 91 +  //
                                 int32_t(s[3] - 33) * 91 + int32_t(s[4] - 33)) *
                        1e7 / 380926);
        // 5-8 Compressed Long XXXX, 23*91*91=190463, Greenwich is 23*180=4140 / 4140*91*91=45*91*91*91+45*91*91
        ll.setLngDeg1e7(int64_t(int32_t(s[5] - 33 - 45) * 91 * 91 * 91 + int32_t(s[6] - 33 - 45) * 91 * 91 +  //
                                int32_t(s[7] - 33) * 91 + int32_t(s[8] - 33)) *
                        1e7 / 190463);

        SymbolCode = s[9];
        // 10-11 Compressed Course/Speed etc.
        // 12 Comp Type T
        s += 13;
      }

      if (s[3] == '/') {  // Course and Speed, pp.27.
        if (isdigit(s[0]) && isdigit(s[1]) && isdigit(s[2])) {
          courseDeg = (s[0] - '0') * 100 + (s[1] - '0') * 10 + (s[2] - '0');
        }
        s += 4;
        if (isdigit(s[0]) && isdigit(s[1]) && isdigit(s[2])) {
          speedKnot = (s[0] - '0') * 100 + (s[1] - '0') * 10 + (s[2] - '0');
        }
        s += 3;

        if (s[0] == '/' && s[4] == '/') {  // Bearing and Number/Range/Quality, pp.30
          s += 8;
        } else if (s[0] == '/' && s[3] == '/' && s[7] == '^' && s[11] == '/' && s[16] == '>' &&
                   s[20] == '&') {  // Storm data, pp. 67.
          s += 24;
          if (s[0] == '%') {  // radius of “whole gale” (i.e. 50 knot) winds (in nautical miles). Optional.
            s += 4;
          }
        } else if (s[0] == 'g' && s[4] == 't') {  // Weather Data, pp. 65.
          s += 4;                                 // Gust
          s += 4;                                 // Temp
          while (true) {
            if (s[0] == 'r') {  // rainfall (in hundredths of an inch) in the last hour.
              s += 4;
            } else if (s[0] == 'p') {  // rainfall (in hundredths of an inch) in the last 24 hours.
              s += 4;
            } else if (s[0] == 'P') {  // rainfall (in hundredths of an inch) since midnight.
              s += 4;
            } else if (s[0] == 'h') {  // humidity (in %. 00 = 100%)
              s += 3;
            } else if (s[0] == 'b') {  // barometric pressure (in tenths of millibars/tenths of hPascal).
              s += 6;
            } else if (s[0] == 'L') {  // luminosity (in watts per square meter) 999 and below.
              s += 3;
            } else if (s[0] == 'l') {  // luminosity (in watts per square meter) 1000 and above
              s += 4;
            } else if (s[0] == 's') {  // snowfall (in inches) in the last 24 hours.
              s += 3;
            } else if (s[0] == '#') {  // raw rain counter
              s += 3;
            } else {
              break;
            }
          }
        }
      } else {
        courseDeg = 0;
        speedKnot = 0;
      }
      break;

    case '`':   // Mic-E Current GPS data
    case '\'':  // Mic-E Old GPS data

      if (MicEDestDecodeDigit(toCall[5], ' ') == ' ') {
        if (MicEDestDecodeDigit(toCall[4], ' ') == ' ') {
          ambiguity = 2;
        } else {
          ambiguity = 1;
        }
      }

      ll.setLatDegMin1e7(
          MicEDestDecodeDigit(toCall[0], 0) * 10 + MicEDestDecodeDigit(toCall[1], 0),
          MicEDestDecodeFlag(toCall[3]) == 1,
          (uint32_t)MicEDestDecodeDigit(toCall[2], 0) * 1e8 + (uint32_t)MicEDestDecodeDigit(toCall[3], 0) * 1e7 +
              (uint32_t)MicEDestDecodeDigit(toCall[4], 0) * 1e6 + (uint32_t)MicEDestDecodeDigit(toCall[5], 0) * 1e5);
      ll.setLngDegMin1e7(MicEHeaderDecodeDegree(s[0], (MicEDestDecodeFlag(toCall[4]) == 1)),
                         MicEDestDecodeFlag(toCall[5]) != 1,
                         (uint32_t)MicEHeaderDecodeMinute(s[1]) * 1e7 + (uint32_t)MicEHeaderDecodeMinute(s[2]) * 1e5);

      switch (MicEDestDecodeFlag(toCall[0]) * 3 * 3 + MicEDestDecodeFlag(toCall[1]) * 3 +
              MicEDestDecodeFlag(toCall[2])) {
        case 9 + 3 + 1:  // M0: Off Duty
          micEMessageType = MicEMsg::M0;
          break;
        case 9 + 3 + 0:  // M1: En Route
          micEMessageType = MicEMsg::M1;
          break;
        case 9 + 0 + 1:  // M2: In Service
          micEMessageType = MicEMsg::M2;
          break;
        case 9 + 0 + 0:  // M3: Returning
          micEMessageType = MicEMsg::M3;
          break;
        case 0 + 3 + 1:  // M4: Committed
          micEMessageType = MicEMsg::M4;
          break;
        case 0 + 3 + 0:  // M5: Special
          micEMessageType = MicEMsg::M5;
          break;
        case 0 + 0 + 1:  // M6: Priority
          micEMessageType = MicEMsg::M6;
          break;
        case 18 + 6 + 2:  // C0: Custom-0
          micEMessageType = MicEMsg::C0;
          break;
        case 18 + 6 + 0:  // C1: Custom-1
          micEMessageType = MicEMsg::C1;
          break;
        case 18 + 0 + 2:  // C2: Custom-2
          micEMessageType = MicEMsg::C2;
          break;
        case 18 + 0 + 0:  // C3: Custom-3
          micEMessageType = MicEMsg::C3;
          break;
        case 0 + 6 + 2:  // C4: Custom-4
          micEMessageType = MicEMsg::C4;
          break;
        case 0 + 6 + 0:  // C5: Custom-5
          micEMessageType = MicEMsg::C5;
          break;
        case 0 + 0 + 2:  // C6: Custom-6
          micEMessageType = MicEMsg::C6;
          break;
        case 0 + 0 + 0:  // Emergency
          micEMessageType = MicEMsg::EMERGENCY;
          break;
        default:  // UNKNOWN
          micEMessageType = MicEMsg::UNKNOWN;
      }

      int sp = s[3] - 28;
      int dc = s[4] - 28;
      speedKnot = sp * 10 + dc / 10;
      speedKnot = speedKnot >= 800 ? speedKnot - 800 : speedKnot;
      int se = s[5] - 28;
      courseDeg = (dc % 10) * 100 + se;
      courseDeg = courseDeg >= 400 ? courseDeg - 400 : courseDeg;

      SymbolCode = s[6];
      SymbolTableIdentifier = s[7];

      s += 8;  // 8

      switch (s[0]) {  // Mic-E Telemetry Data, pp.54.

        case '`':
          if (s[4] == '}') {
            s++;
          } else {  // 2 printable hex telemetry values follow (channels 1 and 3).
            s += 2 * 2;
          }
          break;
        case '\'':  // 5 printable hex telemetry values follow
          s += 5 * 2;
          break;
        case ',':  // 5 binary telemetry values follow.
          s += 5;
          break;
        case 0x1d:  // 5 binary telemetry values follow (Rev. 0 beta units only).
          s += 5;
          break;
      }

      switch (s[0]) {  // Mic-E Status Text, pp.54.
        case '>':      // Kenwood TH-D7
        case ']':      // Kenwood TM-D700
          s++;
      }

      if (s[3] == '}') {  // pp.55-pp.56.
        altitudeMeter = float((int32_t(s[0] - 33) * 91 * 91 + int32_t(s[1] - 33) * 91 + int32_t(s[2] - 33)) - 10000);
        s += 4;  // altitude
      }

      break;
  }

  while (1) {  // 7 APRS DATA EXTENSIONS
    while (s[0] == ' ') {
      s++;
    }
    if (s[0] == 'P' && s[1] == 'H' && s[2] == 'G') {  // PHG
      s += 7;
    } else if (s[0] == 'R' && s[1] == 'N' && s[2] == 'G') {  // RNG
      s += 7;
    } else if (s[0] == 'D' && s[1] == 'F' && s[2] == 'S') {  // DFS
      s += 7;
    } else if (s[0] == '/' && s[1] == 'A' && s[2] == '=') {  // altitude
      s += 3;
      uint32_t altitudeFeet;
      for (altitudeFeet = 0; isdigit(s[0]); s++) {
        altitudeFeet = altitudeFeet * 10 + uint32_t(s[0] - '0');
      }
      altitudeMeter = (float)altitudeFeet / 3.2808;
    } else if (s[0] == '/' && s[8] == 'M' && s[9] == 'H' && s[10] == 'z') {  // like "/433.000MHz"
      s += 11;
    } else if (s[7] == 'M' && s[8] == 'H' && s[9] == 'z') {  // like "433.000MHz"
      s += 10;
    } else {
      break;
    }
  }

  strncpy(message, s, sizeof(message) - 1);
  message[sizeof(message) - 1] = '\0';

  free(s_org);
  return true;
}

// Encode to String.
String APRS::Encode() const {
  double latP, lngP;
  char NS, EW;

  if (ll.getLat() < 0) {
    NS = 'S';
    latP = -ll.getLat();
  } else {
    NS = 'N';
    latP = ll.getLat();
  }
  if (ll.getLng() <= 0) {
    EW = 'W';
    lngP = -ll.getLng();
  } else {
    EW = 'E';
    lngP = ll.getLng();
  }

  uint8_t latDeg = latP;
  uint8_t lngDeg = lngP;

  float latMin = (latP - latDeg) * 60;
  float lngMin = (lngP - lngDeg) * 60;

  size_t maxMsgLen;
  switch (DataType) {
    case '!':  // pp.32.
    case '=':  // pp.32.
    case '/':  // pp.32.
    case '@':  // pp.32.
      maxMsgLen = 43;
      break;
    default:
      maxMsgLen = 0;
  }

  char msg[128];
  msg[0] = 0;

  if (DataType == '!') {
    switch (ambiguity) {  // pp.24
      case 4:
        sprintf(msg, "%c%02d  .  %c%c%03d  .  %c%c", DataType, latDeg, NS, SymbolTableIdentifier, lngDeg, EW,
                SymbolCode);
        break;
      case 3:
        sprintf(msg, "%c%02d%01.0f .  %c%c%03d%01.0f .  %c%c", DataType, latDeg, latMin / 10, NS, SymbolTableIdentifier,
                lngDeg, lngMin / 10, EW, SymbolCode);
        break;
      case 2:
        sprintf(msg, "%c%02d%02.0f.  %c%c%03d%02.0f.  %c%c", DataType, latDeg, latMin, NS, SymbolTableIdentifier,
                lngDeg, lngMin, EW, SymbolCode);
        break;
      case 1:
        sprintf(msg, "%c%02d%02.1f %c%c%03d%02.1f %c%c", DataType, latDeg, latMin, NS, SymbolTableIdentifier, lngDeg,
                lngMin, EW, SymbolCode);
        break;
      case 0:
      default:
        sprintf(msg, "%c%02d%02.2f%c%c%03d%02.2f%c%c", DataType, latDeg, latMin, NS, SymbolTableIdentifier, lngDeg,
                lngMin, EW, SymbolCode);
    }
    strncat(msg, message, maxMsgLen);
  }

  return String(msg);
}

String APRS::Symbol() const {
  static const char *primarySymbolTable[] = {
      // APPENDIX 2: THE APRS SYMBOL TABLES, pp.104.
      "Police, Sheriff",
      "",  // [reserved]
      "Digi",
      "Phone",
      "DX Cluster",
      "HF Gateway",
      "Small Aircraft",
      "Mobile Satellite Groundstation",
      "",
      "Snowmobile",
      "Red Cross",
      "Boy Scouts",
      "House QTH (VHF)",
      "X",
      "Dot",
      "0",
      "1",
      "2",
      "3",
      "4",
      "5",
      "6",
      "7",
      "8",
      "9",
      "Fire",
      "Campground",
      "Motorcycle",
      "Railroad Engine",
      "Car",
      "File Server",
      "Hurricane Future Prediction (dot)",
      "Aid Station",
      "BBS",
      "Canoe",
      "",
      "Eyeball (eye catcher)",
      "",
      "Grid Square (6-character) ",
      "Hotel (blue bed icon)",
      "TCP/IP",
      "",
      "School",
      "",
      "MacAPRS",
      "NTS Station",
      "Balloon",
      "Police",
      "",
      "Recreational Vehicle",
      "Space Shuttle ",
      "SSTV",
      "Bus",
      "ATV",
      "National Weather Service Site",
      "Helicopter",
      "Yacht (sail boat)",
      "WinAPRS",  // Z
      "Jogger",
      "Triangle (DF)",
      "PBBS",
      "Large Aircraft",
      "Weather Station (blue)",
      "Dish Antenna",
      "Ambulance",  // a
      "Bicycle",
      "",
      "Dual Garage (Fire Department)",
      "Horse (equestrian)",
      "Fire Truck",
      "Glider",
      "Hospital",
      "IOTA",
      "Jeep",
      "Truck",
      "",
      "Mic-repeater",
      "Node",
      "Emergency Operations Center",
      "Rover (puppy dog)",
      "Grid Square shown above 128m",
      "Antenna",
      "Ship (power boat)",
      "Truck Stop",
      "Truck (18-wheeler)",
      "Van",
      "Water Station",
      "X-APRS (Unix)",
      "Yagi at QTH",  // y
      "",
      "",
      "",  //[Reserved — TNC Stream Switch]
      "",
      "",  //[Reserved — TNC Stream Switch]
  };

  static const char *alternateSymbolTable[] = {
      "Emergency",
      "",  // [reserved]
      "Digi (green star)",
      "Bank or ATM",
      "",
      "HF Gateway (diamond)",
      "Crash Site",
      "Cloudy",
      "",
      "Snow",
      "Church",
      "Girl Scouts",
      "House (HF)",
      "Unknown/indeterminate position",
      "",
      "Circle",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "",
      "Gas Station (blue pump)",
      "Hail",
      "Park/Picnic Area",
      "NWS Advisory (gale flag)",
      "",
      "Car",
      "Information Kiosk (blue box with ?)",
      "Hurricane/Tropical Storm",
      "Box",
      "Blowing Snow",
      "Coastguard",
      "Drizzle",
      "Smoke",
      "Freezing Rain",
      "Snow Shower",
      "Haze",
      "Rain Shower",
      "Lightning",
      "Kenwood",
      "Lighthouse",
      "",
      "Navigation Buoy",
      "",
      "Parking",
      "Earthquake",
      "Restaurant",
      "Satellite/PACsat",
      "Thunderstorm",
      "Sunny",
      "VORTAC Nav Aid",
      "NWS Site",
      "Pharmacy Rx",
      "",
      "",  // Z
      "Wall Cloud",
      "",
      "",
      "Aircraft",
      "WX Stn with digi (green)",
      "Rain",
      "(A=ARRL, R=RACES etc)",  // a
      "Blowing Dust/Sand",
      "Civil Defense (RACES)",
      "DX Spot (from callsign prefix)",
      "Sleet",
      "Funnel Cloud",
      "Gale Flags",
      "Ham Store",
      "Indoor short range digi",
      "Work Zone (steam shovel)",
      "",
      "Area Symbols (box, circle, etc)",
      "Value Signpost",
      "Triangle",
      "Small Circle",
      "Partly Cloudy",
      "",
      "Restrooms",
      "Ship/Boat (top view) ",
      "Tornado",
      "Truck",
      "Van",
      "",
      "",
      "",  // y
      "",
      "Fog",
      "",  //[Reserved — TNC Stream Switch]
      "",
      "",  //[Reserved — TNC Stream Switch]
  };

  switch (SymbolTableIdentifier) {
    case '/':
      return primarySymbolTable[SymbolCode - 33];
      break;
    case '\\':
      return alternateSymbolTable[SymbolCode - 33];
      break;
    default:  // http://www.aprs.org/symbols/symbols-new.txt
      switch (SymbolCode) {
        case '#':  // DIGIPEATERS
          switch (SymbolTableIdentifier) {
            case '1':
              return "WIDE1-1 digipeater";
            case 'A':
              return "Alternate input digipeater";
            case 'E':
              return "Emergency powered (assumed full normal digi)";
            case 'I':
              return "I-gate equipped digipeater";
            case 'L':
              return "WIDEn-N with path length trapping";
            case 'P':
              return "PacComm";
            case 'S':
              return "SSn-N digipeater (includes WIDEn-N)";
            case 'X':
              return "eXperimental digipeater";
            case 'V':
              return "Viscous";
            case 'W':
              return "WIDEn-N, SSn-N and Trapping";
          }
        case '&':  // GATEWAYS
          switch (SymbolTableIdentifier) {
            case 'I':
              return "Igate Generic";
            case 'L':
              return "Lora Igate";
            case 'R':
              return "Receive only IGate";
            case 'P':
              return "PSKmail node";
            case 'T':
              return "TX igate with path set to 1 hop only";
            case 'W':
              return "WIRES-X as opposed to W0 for WiresII";
            case '2':
              return "TX igate with path set to 2 hops";
            case 'D':
              return "Dstar Repeater Igate";
          }
        default:
          return String(alternateSymbolTable[SymbolCode - 33]) + " with Overlay " + SymbolTableIdentifier;
          break;
      }
      break;
  }
  return "";
}