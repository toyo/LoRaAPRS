#ifndef _LATLNG_H
#define _LATLNG_H

#include <Arduino.h>

class LatLng {
  int32_t lat_1e7;  // lat_1e7 = LatitudeDegree * 1e7.
  int32_t lng_1e7;  // lng_1e7 = LongitudeDegree * 1e7.

 public:
  LatLng() {}

  virtual bool setup() { return true; }
  virtual bool loop() { return true; }

  virtual bool isValid() const { return true; }

  virtual int32_t getLat1e7() { return lat_1e7; }
  virtual int32_t getLng1e7() { return lng_1e7; }

  virtual void setLatDeg1e7(int32_t latDeg1e7) { lat_1e7 = latDeg1e7; }
  virtual void setLngDeg1e7(int32_t lngDeg1e7) { lng_1e7 = lngDeg1e7; }

  double getLat() { return getLat1e7() / 1e7; }
  double getLng() { return getLng1e7() / 1e7; }

  void set(double _lat, double _lng) {
    setLatDeg1e7(_lat * 1e7);
    setLngDeg1e7(_lng * 1e7);
  }

  void setLatDegMin1e7(int latDeg, bool north, uint32_t latMin1e7 = 0) {
    uint32_t _lat1e7 = latDeg * 1e7 + (latMin1e7 / 60);
    setLatDeg1e7(north ? _lat1e7 : -_lat1e7);
  }
  void setLngDegMin1e7(int lngDeg, bool east, uint32_t lngMin1e7 = 0) {
    uint32_t _lng1e7 = lngDeg * 1e7 + (lngMin1e7 / 60);
    setLngDeg1e7(east ? _lng1e7 : -_lng1e7);
  }

  static double radiansFrom1e7(int32_t deg1e7) { return double(deg1e7) / 1e7 * DEG_TO_RAD; }
  static int32_t subtraction1e7(int32_t degx1e7, int32_t degy1e7);

  static uint32_t distancecm1e7(int32_t lat1e7, int32_t lng1e7, int32_t _lat1e7, int32_t _lng1e7);
  static float bearing1e7(int32_t lat1e7, int32_t lng1e7, int32_t _lat1e7, int32_t _lng1e7);

  uint32_t distancecmFrom(LatLng &ll) {
    return distancecm1e7(getLat1e7(), getLng1e7(), ll.getLat1e7(), ll.getLng1e7());
  }
  float bearingFrom(LatLng &ll) { return bearing1e7(getLat1e7(), getLng1e7(), ll.getLat1e7(), ll.getLng1e7()); }
};

#endif  //_LATLNG_H