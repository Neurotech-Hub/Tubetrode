#include "Tubetrode.h"
#include "CalibrationData.h"

#define FLT_MAX 3.4e8

template <size_t N, size_t M>
constexpr size_t getRowCount(float (&)[N][M])
{
  return N;
}

Tubetrode::Tubetrode(uint8_t sensorBlock1Addr, uint8_t sensorBlock2Addr, uint8_t enablePin) : ADS(sensorBlock1Addr), ADS2(sensorBlock2Addr)
{
  this->_enablePin = enablePin;
  // Initialize the position buffer with zeros
  for (int i = 0; i < bufferSize; i++)
  {
    positionBuffer[i] = 0.0;
  }
}

void Tubetrode::begin()
{
  pinMode(_enablePin, OUTPUT);
  digitalWrite(_enablePin, HIGH);
  Wire.begin();
  ADS.begin();
  ADS.setGain(0);
  ADS.setDataRate(4); //  0 = slow   4 = medium   7 = fast

  ADS2.begin();
  ADS2.setDataRate(4); //  0 = slow   4 = medium   7 = fast
  ADS2.setGain(0);
  this->_voltageFactor = ADS.toVoltage(1); // voltage factor

  // call once to warm everything up?
  // float tempSensorValues[8];
  // readRawSensors(tempSensorValues, false);
  // in the future think about disabling by default
  // digitalWrite(_enablePin, LOW);
}

bool Tubetrode::isReady()
{
  bool isReady = !ADS.isReady() | !ADS2.isReady();
  return isReady;
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

  const size_t rowCount = getRowCount(calibrationData);
  float minimized[rowCount];
  int minIndex = 0;
  float minSum = FLT_MAX; // Set initial minimum to the largest possible float value

  for (size_t i = 0; i < rowCount; i++)
  {
    float sum = 0;
    for (int j = 0; j < 8; j++)
    {
      sum += fabs(calibrationData[i][j + 1] - sensorValues[j]); // Calculate absolute difference and add to sum
    }
    minimized[i] = sum;

    if (sum < minSum)
    {
      minSum = sum;
      minIndex = i;
    }
  }

  float positionEstimate = calibrationData[minIndex][0]; // Get position estimate from calibration data

  // Store the new position estimate in the buffer and update the index
  positionBuffer[bufferIndex] = positionEstimate;
  bufferIndex = (bufferIndex + 1) % bufferSize; // Wrap around the buffer index

  return positionEstimate;
}

float Tubetrode::averagePosition()
{
  float sum = 0.0;
  for (int i = 0; i < bufferSize; i++)
  {
    sum += positionBuffer[i];
  }
  return sum / bufferSize;
}