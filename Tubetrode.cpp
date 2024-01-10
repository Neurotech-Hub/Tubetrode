#include "Arduino.h"
#include "Tubetrode.h"
#include "ADS1X15.h"
#include "CalibrationData.h"

Tubetrode::Tubetrode(uint8_t sensorBlock1Addr, uint8_t sensorBlock2Addr, uint8_t enablePin) : ADS(sensorBlock1Addr), ADS2(sensorBlock2Addr)
{
  this->_enablePin = enablePin;
}

void Tubetrode::begin()
{
  pinMode(_enablePin, OUTPUT);
  digitalWrite(_enablePin, HIGH);
  ADS.begin();
  ADS2.begin();
  ADS.setGain(0);
  ADS2.setGain(0);
  this->_voltageFactor = ADS.toVoltage(1); // voltage factor
}

void Tubetrode::readRawSensors(float *rawSensorValues, bool toVolts)
{
  for (int i = 0; i < 8; i++)
  {
    if (i < 4)
    {
      rawSensorValues[i] = ADS.readADC(i);
    }
    else
    {
      rawSensorValues[i] = ADS2.readADC(i - 4);
    }
    if (toVolts)
    {
      rawSensorValues[i] *= _voltageFactor;
    }
  }
}

float Tubetrode::estimatePosition()
{
  float sensorValues[8];
  readRawSensors(sensorValues, true); // Read sensor values in volts

  float positionEstimate = 0.0; // Variable to hold the estimated position

  for (int i = 0; i < 8; i++)
  {
    // Compare each sensor value to the calibration value
    // Custom logic here to calculate the position estimate
    float difference = sensorValues[i] - calibrationData[i];
    // Update positionEstimate based on the difference and your algorithm
  }

  // Return the calculated position estimate
  return positionEstimate;
}