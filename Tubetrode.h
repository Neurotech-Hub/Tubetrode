#ifndef Tubetrode_h
#define Tubetrode_h

#include "Arduino.h"
#include "ADS1X15.h"

// Sensor block address definitions
#define ADDR_GND 0x48
#define ADDR_VDD 0x49
#define ADDR_SDA 0x4A
#define ADDR_SCL 0x4B

class Tubetrode {
  public:
    Tubetrode(uint8_t sensorBlock1Addr, uint8_t sensorBlock2Addr, uint8_t enablePin);
    void begin();
    void readRawSensors(float *rawSensorValues, bool toVolts = true);
    float estimatePosition();

  private:
    ADS1115 ADS, ADS2;
    uint8_t _enablePin;
    float _voltageFactor;
    float calibrationData[8]; // Example calibration data array
};

#endif