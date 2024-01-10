#include <Tubetrode.h>

HALL_EN_GPIO = 13; // !! User defined, turns on Tubetrode module

// Initialize Tubetrode with sensor addresses and enable pin
Tubetrode tubetrode(ADDR_SCL, ADDR_SDA, HALL_EN_GPIO);

// Array to hold sensor values
float sensorValues[8];

void setup()
{
    // Start serial communication
    Serial.begin(115200);

    // Initialize Tubetrode
    tubetrode.begin();
}

void loop()
{
    // Read raw sensor values
    tubetrode.readRawSensors(sensorValues, false); // Read raw data
    Serial.println("Raw Sensor Values:");
    printSensorValues();

    // Read sensor values converted to volts
    tubetrode.readRawSensors(sensorValues, true); // Convert to volts
    Serial.println("Sensor Values in Volts:");
    printSensorValues();

    // Wait for a short period
    delay(1000);
}

void printSensorValues()
{
    for (int i = 0; i < 8; i++)
    {
        Serial.print(sensorValues[i]);
        Serial.print("\t"); // Tab space for better readability
    }
    Serial.println(); // New line after printing all values
}
