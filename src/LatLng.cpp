#include "LatLng.h"

int32_t LatLng::subtraction1e7(int32_t degx1e7, int32_t degy1e7) {
  int64_t subtr = int64_t(degx1e7) - int64_t(degy1e7);
  if (subtr > 180e7) {
    subtr -= 360e7;
  }
  if (subtr < -180e7) {
    subtr += 360e7;
  }

  return subtr;
}

uint32_t LatLng::distancecm1e7(int32_t lat1e7, int32_t lng1e7, int32_t _lat1e7, int32_t _lng1e7) {
  // returns distance in centi-meters between two positions,
  double sdlng = sin(radiansFrom1e7(subtraction1e7(_lng1e7, lng1e7)));
  double cdlng = cos(radiansFrom1e7(subtraction1e7(_lng1e7, lng1e7)));
  double slat = sin(radiansFrom1e7(lat1e7));
  double clat = cos(radiansFrom1e7(lat1e7));
  double _slat = sin(radiansFrom1e7(_lat1e7));
  double _clat = cos(radiansFrom1e7(_lat1e7));

  return atan2(sqrt(sq((clat * _slat) - (slat * _clat * cdlng)) + sq(_clat * sdlng)),
               (slat * _slat) + (clat * _clat * cdlng)) *
         6378137 * 100;
}

float LatLng::bearing1e7(int32_t lat1e7, int32_t lng1e7, int32_t _lat1e7, int32_t _lng1e7) {
  // returns course in degrees (North=0, West=270) from position 1 to position 2,
  double sdlng = sin(radiansFrom1e7(subtraction1e7(_lng1e7, lng1e7)));
  double cdlng = cos(radiansFrom1e7(subtraction1e7(_lng1e7, lng1e7)));
  double slat = sin(radiansFrom1e7(lat1e7));
  double clat = cos(radiansFrom1e7(lat1e7));
  double _slat = sin(radiansFrom1e7(_lat1e7));
  double _clat = cos(radiansFrom1e7(_lat1e7));

  float deg = degrees(atan2(sdlng * _clat, clat * _slat - slat * _clat * cdlng));
  if (deg < 0) {
    deg += 360;
  }
  return deg;
}