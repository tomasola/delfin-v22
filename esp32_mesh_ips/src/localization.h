#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <Arduino.h>
#include <math.h>

class KalmanFilter {
public:
  KalmanFilter(float q, float r, float p, float initial_value) {
    _q = q;
    _r = r;
    _p = p;
    _x = initial_value;
  }

  float update(float measurement) {
    // Prediction
    _p = _p + _q;

    // Measurement update
    float k = _p / (_p + _r);
    _x = _x + k * (measurement - _x);
    _p = (1 - k) * _p;

    return _x;
  }

private:
  float _q; // Process noise
  float _r; // Measurement noise
  float _p; // Estimation error
  float _x; // Value
};

// RSSI to Meters: distance = 10^((Measured Power - RSSI) / (10 * N))
// Measured Power (A) typically -59 at 1 meter. N is path-loss exponent (2 to
// 4).
inline float rssiToMeters(float rssi, float measurePower = -59, float n = 2.0) {
  if (rssi == 0)
    return -1.0;
  return pow(10, (measurePower - rssi) / (10 * n));
}

struct Point {
  float x, y;
};

// Trilateration simple para 3 nodos
inline Point trilaterate(Point p1, float d1, Point p2, float d2, Point p3,
                         float d3) {
  float x, y;
  float A = 2 * p2.x - 2 * p1.x;
  float B = 2 * p2.y - 2 * p1.y;
  float C =
      d1 * d1 - d2 * d2 - p1.x * p1.x + p2.x * p2.x - p1.y * p1.y + p2.y * p2.y;
  float D = 2 * p3.x - 2 * p2.x;
  float E = 2 * p3.y - 2 * p2.y;
  float F =
      d2 * d2 - d3 * d3 - p2.x * p2.x + p3.x * p3.x - p2.y * p2.y + p3.y * p3.y;

  x = (C * E - F * B) / (E * A - B * D);
  y = (C * D - A * F) / (B * D - A * E);

  return {x, y};
}

#endif
