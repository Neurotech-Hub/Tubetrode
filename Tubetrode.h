#ifndef Tubetrode_h
#define Tubetrode_h

#include "ADS1X15.h"
#include "Wire.h"

// Sensor block address definitions
#define ADDR_GND 0x48
#define ADDR_VDD 0x49
#define ADDR_SDA 0x4A
#define ADDR_SCL 0x4B

class Tubetrode
{
public:
  Tubetrode(uint8_t sensorBlock1Addr, uint8_t sensorBlock2Addr, uint8_t enablePin);
  void begin();
  void readRawSensors(float *rawSensorValues, bool toVolts = true);
  bool isReady();
  float estimatePosition();
  float averagePosition();

private:
  ADS1115 ADS,
      ADS2;
  uint8_t _enablePin;
  float _voltageFactor;
  static const int bufferSize = 10; // Size of the position estimate buffer
  float positionBuffer[bufferSize]; // Buffer to store the last position estimates
  int bufferIndex = 0;              // Index for the next position estimate to be stored
};

#endif
