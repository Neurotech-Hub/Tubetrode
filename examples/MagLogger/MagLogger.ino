///////////////////////////////////////////////////////////////
// Include libraries
///////////////////////////////////////////////////////////////
#include <SPI.h>
#include <SD.h>
#include <Tubetrode.h>

#define LOOP_DELAY 100 // 9828

///////////////////////////////////////////////////////////////
// Initialize Tubetrode with sensor addresses and enable pin
///////////////////////////////////////////////////////////////
#define HALL_EN_GPIO 5 // !! User defined, turns on Tubetrode module
Tubetrode tubetrode1(ADDR_SCL, ADDR_SDA, HALL_EN_GPIO);
Tubetrode tubetrode2(ADDR_VDD, ADDR_GND, HALL_EN_GPIO);

///////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////
const int chipSelect = 4; // SPI needs a "chip select" pin for each device, in this case the SD card
char filename[16];        // make a "char" type variable called "filename", containing 16 characters
float sensorValues[8];
float measuredvbat;

void setup()
{
  ///////////////////////////////////////////////////////////////
  // Set pinmodes
  ///////////////////////////////////////////////////////////////
  pinMode(8, OUTPUT);  // green LED
  pinMode(13, OUTPUT); // red LED

  ///////////////////////////////////////////////////////////////
  // Start serial communication
  ///////////////////////////////////////////////////////////////
  Serial.begin(115200);
  long startTime = millis();
  while ((millis() - startTime) < 5) // wait for 5 seconds
  {
    digitalWrite(13, ~digitalRead(13)); // toggle
    delay(100);
    if (Serial)
    {
      break; // Exit the loop if a serial connection is established
    }
  }

  ///////////////////////////////////////////////////////////////
  // Initialize Tubetrode
  ///////////////////////////////////////////////////////////////
  Serial.print("Tubetrode... ");
  tubetrode1.begin();
  tubetrode2.begin();
  Serial.println("initialized.");

  ///////////////////////////////////////////////////////////////
  // Check if an SD card is present and can be initialized:
  ///////////////////////////////////////////////////////////////
  digitalWrite(13, LOW); // Red LED off if logging fine
  if (!SD.begin(chipSelect))
  {
    Serial.println("Card failed, or not present");
    digitalWrite(13, HIGH); // Red LEDs on solid means no logging
  }
  Serial.println("card initialized.");

  //////////////////////////////////////////////////////////////////////////
  // Generate a unique filename (example from: https://forum.arduino.cc/index.php?topic=372248.0)
  //////////////////////////////////////////////////////////////////////////
  int n = 0;
  snprintf(filename, sizeof(filename), "tube%03d.csv", n); // includes a three-digit sequence number in the file name
  while (SD.exists(filename))
  {
    n++;
    snprintf(filename, sizeof(filename), "tube%03d.csv", n);
  }
  Serial.print("New file created: ");
  Serial.println(filename);

  ///////////////////////////////////////////////////////////////
  // Write header on data file
  ///////////////////////////////////////////////////////////////
  File dataFile = SD.open(filename, FILE_WRITE); // open the file with the name "filename"
  if (dataFile)
  {                                                                                         // if the file is available...
    Serial.println("Millis,vBAT,Mag1,Mag2,Mag3,Mag4,Mag5,Mag6,Mag7,Mag8,EstPos,TrodeId");   // Write dataString to the SD card
    dataFile.println("Millis,vBAT,Mag1,Mag2,Mag3,Mag4,Mag5,Mag6,Mag7,Mag8,EstPos,TrodeId"); // Write dataString to the SD card
    dataFile.close();                                                                       // Close the file
  }
}

void loop()
{
  ///////////////////////////////////////////////////////////////
  // Read raw sensor values and estimate magnet position
  ///////////////////////////////////////////////////////////////
  ReadBatteryLevel();
  float timeStamp = (millis() / 1000.0);

  tubetrode1.readRawSensors(sensorValues, true); // Read raw data
  float pos1 = tubetrode1.estimatePosition();
  String dataString1 = "";                                                                                                                                                                                                                                                                                                                                                             // make a string for assembling the data to log
  dataString1 = String(timeStamp) + "," + String(measuredvbat) + "," + String(sensorValues[0], 3) + "," + String(sensorValues[1], 3) + "," + String(sensorValues[2], 3) + "," + String(sensorValues[3], 3) + "," + String(sensorValues[4], 3) + "," + String(sensorValues[5], 3) + "," + String(sensorValues[6], 3) + "," + String(sensorValues[7], 3) + "," + String(pos1, 3) + ",1"; // <-tubeTrodeId

  tubetrode2.readRawSensors(sensorValues, true); // Read raw data
  float pos2 = tubetrode2.estimatePosition();
  String dataString2 = "";                                                                                                                                                                                                                                                                                                                                                             // make a string for assembling the data to log
  dataString2 = String(timeStamp) + "," + String(measuredvbat) + "," + String(sensorValues[0], 3) + "," + String(sensorValues[1], 3) + "," + String(sensorValues[2], 3) + "," + String(sensorValues[3], 3) + "," + String(sensorValues[4], 3) + "," + String(sensorValues[5], 3) + "," + String(sensorValues[6], 3) + "," + String(sensorValues[7], 3) + "," + String(pos2, 3) + ",2"; // <-tubeTrodeId

  // show est. ml remaining
  Serial.println(String(pos1, 2) + " - " + String(pos2, 2));
  // debugging sensor values
  // for (int ii = 0; ii < 8; ii++) {
  //   Serial.print(String(sensorValues[ii], 2));
  //   if (ii < 7) {
  //     Serial.print(",");
  //   }
  // }
  // Serial.println("");

  ///////////////////////////////////////////////////////////////
  // Write data to card
  ///////////////////////////////////////////////////////////////
  File dataFile = SD.open(filename, FILE_WRITE); // open the file with the name "filename"
  if (dataFile)
  { // if the file is available...
    digitalWrite(8, HIGH);
    dataFile.println(dataString1); // Write dataString to the SD card
    dataFile.println(dataString2); // Write dataString to the SD card
    dataFile.close();              // Close the file
    // Serial.println(dataString);
    delay(100);
    digitalWrite(8, LOW);
  }

  ///////////////////////////////////////////////////////////////
  // Wait for a short period (should improve this to make an actual user settable sampling rate)
  ///////////////////////////////////////////////////////////////
  delay(LOOP_DELAY);
}

void ReadBatteryLevel()
{
  measuredvbat = analogRead(A7);
  measuredvbat *= 2;    // we divided by 2, so multiply back
  measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024; // convert to voltage
}
