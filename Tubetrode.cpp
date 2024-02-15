#include "Tubetrode.h"
#include "CalibrationData.h"

#define NUM_CAL_ROWS 274 // must match calibration files
#define FLT_MAX 3.4e8

Tubetrode::Tubetrode(uint8_t sensorBlock1Addr, uint8_t sensorBlock2Addr, uint8_t enablePin) : ADS(sensorBlock1Addr), ADS2(sensorBlock2Addr)
{
  this->_enablePin = enablePin;
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

  float positionEstimate = 0.0; // Variable to hold the estimated position
  int closestRow = findClosestRankRow(sensorValues, idealRank, NUM_CAL_ROWS);
  int nearestSensor = useSensor[closestRow];

  float lookupCurve[NUM_CAL_ROWS];
  float subCurve[NUM_CAL_ROWS];
  float minVal = FLT_MAX;
  int useRow = -1;

  // Create lookupCurve and subCurve
  for (int i = 0; i < NUM_CAL_ROWS; i++)
  {
    lookupCurve[i] = abs(calibrationData[i][nearestSensor] - sensorValues[nearestSensor]);
    subCurve[i] = lookupCurve[i];
  }

  // Modify subCurve based on useSensor
  for (int i = 0; i < NUM_CAL_ROWS; i++)
  {
    if (useSensor[i] != nearestSensor)
    {
      subCurve[i] = FLT_MAX; // Use FLT_MAX instead of NaN
    }
  }

  // Find min value in subCurve
  for (int i = 0; i < NUM_CAL_ROWS; i++)
  {
    if (subCurve[i] < minVal)
    {
      minVal = subCurve[i];
      useRow = i;
    }
  }

  // Get positionEstimate from calibrationData
  if (useRow != -1)
  {
    positionEstimate = calibrationData[useRow][0];
  }

  return positionEstimate;
}

void Tubetrode::sortAndGetRanks(float array[], int length, int ranks[])
{
  // Temporary array to store the value and original index
  struct
  {
    float value;
    int originalIndex;
  } temp[length];

  // Initialize temporary array
  for (int i = 0; i < length; i++)
  {
    temp[i].value = array[i];
    temp[i].originalIndex = i;
  }

  // Sort the temporary array based on the sensor values
  for (int i = 0; i < length - 1; i++)
  {
    for (int j = 0; j < length - i - 1; j++)
    {
      if (temp[j].value > temp[j + 1].value)
      {
        // Swap elements
        auto tmp = temp[j];
        temp[j] = temp[j + 1];
        temp[j + 1] = tmp;
      }
    }
  }

  // Extract the ranks
  for (int i = 0; i < length; i++)
  {
    ranks[temp[i].originalIndex] = i;
  }
}

int Tubetrode::findClosestRankRow(float sensorValues[], int idealRank[][8], int numIdealRanks)
{
  int sensorRanks[8];
  sortAndGetRanks(sensorValues, 8, sensorRanks);

  int minRankRow = 0;
  float minDifferenceSum = FLT_MAX; // arbitrary large number

  for (int i = 0; i < numIdealRanks; i++) // Loop through each row of idealRank
  {
    float differenceSum = 0;
    for (int j = 0; j < 8; j++) // Assuming each row in idealRank has 8 elements
    {
      differenceSum += abs(idealRank[i][j] - sensorRanks[j]);
    }
    if (differenceSum < minDifferenceSum)
    {
      minDifferenceSum = differenceSum;
      minRankRow = i;
    }
  }

  return minRankRow;
}
