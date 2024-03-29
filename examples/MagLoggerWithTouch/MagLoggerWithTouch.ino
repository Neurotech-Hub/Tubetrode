///////////////////////////////////////////////////////////////
//Include libraries
///////////////////////////////////////////////////////////////
#include <SPI.h>
#include <SD.h>
#include "Tubetrode.h"
#include "Adafruit_FreeTouch.h"
#include "ArduinoLowPower.h"

/********************************************************
  Initialize FreeTouch
********************************************************/
Adafruit_FreeTouch qt_1 = Adafruit_FreeTouch(A1, OVERSAMPLE_64, RESISTOR_20K, FREQ_MODE_NONE);
Adafruit_FreeTouch qt_2 = Adafruit_FreeTouch(A2, OVERSAMPLE_64, RESISTOR_20K, FREQ_MODE_NONE);
int sensitivity = 5;  //modify this to change sensitivity to touch
int baseline1;
int baseline2;
#define LeftSip A1
#define RightSip A2
#define RED_LED 13
#define GREEN_LED 8
int LeftCount = 0;
int RightCount = 0;
int wakecounter = 0;

///////////////////////////////////////////////////////////////
// Initialize Tubetrode with sensor addresses and enable pin
///////////////////////////////////////////////////////////////
#define HALL_EN_GPIO 5  // !! User defined, turns on Tubetrode module
Tubetrode tubetrode(ADDR_SCL, ADDR_SDA, HALL_EN_GPIO);

///////////////////////////////////////////////////////////////
// Variables
///////////////////////////////////////////////////////////////
const int chipSelect = 4;  // SPI needs a "chip select" pin for each device, in this case the SD card
char filename[16];         // make a "char" type variable called "filename", containing 16 characters
float sensorValues[8];
float measuredvbat;

void setup() {
  ///////////////////////////////////////////////////////////////
  // Set pinmodes
  ///////////////////////////////////////////////////////////////
  pinMode(8, OUTPUT);   //green LED
  pinMode(13, OUTPUT);  //red LED
  pinMode(LeftSip, INPUT_PULLDOWN);
  pinMode(RightSip, INPUT_PULLDOWN);

  /********************************************************
    Start FreeTouch
  ********************************************************/
  if (!qt_1.begin())
    Serial.println("Failed to begin qt on pin A0");

  if (!qt_2.begin())
    Serial.println("Failed to begin qt on pin A1");
  delay(1000);
  zerosensors();

  ///////////////////////////////////////////////////////////////
  // Start serial communication
  ///////////////////////////////////////////////////////////////
  Serial.begin(115200);
  delay(500);

  ///////////////////////////////////////////////////////////////
  // Initialize Tubetrode
  ///////////////////////////////////////////////////////////////
  Serial.print("Tubetrode... ");
  tubetrode.begin();
  Serial.println("initialized.");

  ///////////////////////////////////////////////////////////////
  // Check if an SD card is present and can be initialized:
  ///////////////////////////////////////////////////////////////
  digitalWrite(13, LOW);  //Red LED off if logging fine
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    digitalWrite(13, HIGH);  //Red LEDs on solid means no logging
  }
  Serial.println("card initialized.");

  //////////////////////////////////////////////////////////////////////////
  // Generate a unique filename (example from: https://forum.arduino.cc/index.php?topic=372248.0)
  //////////////////////////////////////////////////////////////////////////
  int n = 0;
  snprintf(filename, sizeof(filename), "tube%03d.csv", n);  // includes a three-digit sequence number in the file name
  while (SD.exists(filename)) {
    n++;
    snprintf(filename, sizeof(filename), "tube%03d.csv", n);
  }
  Serial.print("New file created: ");
  Serial.println(filename);

  ///////////////////////////////////////////////////////////////
  // Write header on data file
  ///////////////////////////////////////////////////////////////
  File dataFile = SD.open(filename, FILE_WRITE);                                                          // open the file with the name "filename"
  if (dataFile) {                                                                                         // if the file is available...
    Serial.println("Millis,vBAT,LeftCount,RightCount,Mag1,Mag2,Mag3,Mag4,Mag5,Mag6,Mag7,Mag8");    // Write dataString to the SD card
    dataFile.println("Millis,vBAT,LeftCount,RightCount,Mag1,Mag2,Mag3,Mag4,Mag5,Mag6,Mag7,Mag8");  // Write dataString to the SD card
    dataFile.close();                                                                                     // Close the file
  }
}

void loop() {
  CheckSippers();
  GoToSleep();
}




void ReadBatteryLevel() {
  measuredvbat = analogRead(A7);
  measuredvbat *= 2;     // we divided by 2, so multiply back
  measuredvbat *= 3.3;   // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024;  // convert to voltage
}

void logdata() {
  //////////////////////////////////////////////////////////////
  // Read raw sensor values and estimate magnet position
  ///////////////////////////////////////////////////////////////
  tubetrode.readRawSensors(sensorValues, true);  // Read raw data
  float pos = tubetrode.estimatePosition();

  ///////////////////////////////////////////////////////////////
  // Create a data string to write to the card
  ///////////////////////////////////////////////////////////////
  ReadBatteryLevel();
  String dataString = "";  // make a string for assembling the data to log
  float timeStamp = (millis() / 1000.0);
  dataString = String(timeStamp) + 
  "," + String(measuredvbat) +
  ',' + String(LeftCount) + 
  ',' + String(RightCount) +  // put everything together to write to the card
  "," + String(sensorValues[0]) + 
  "," + String(sensorValues[1]) + 
  "," + String(sensorValues[2]) + 
  "," + String(sensorValues[3]) + 
  "," + String(sensorValues[4]) + 
  "," + String(sensorValues[5]) + 
  "," + String(sensorValues[6]) + 
  "," + String(sensorValues[7]);

  ///////////////////////////////////////////////////////////////
  //Write data to card
  ///////////////////////////////////////////////////////////////
  File dataFile = SD.open(filename, FILE_WRITE);  // open the file with the name "filename"
  if (dataFile) {                                 // if the file is available...
    digitalWrite(8, HIGH);
    dataFile.println(dataString);  // Write dataString to the SD card
    dataFile.close();              // Close the file
    Serial.println(dataString);
    delay(100);
    digitalWrite(8, LOW);
  }
}

void zerosensors() {
  baseline1 = qt_1.measure();
  baseline2 = qt_2.measure();
}

/********************************************************
  Check Sippers
********************************************************/
void CheckSippers() {
  //If left sipper is triggered
  if ((qt_1.measure() - baseline1) > sensitivity) {
    unsigned long start = millis();
    LeftCount++;
    digitalWrite(RED_LED, HIGH);
    delay(50);
    digitalWrite(RED_LED, LOW);
    Serial.print("Left: ");
    Serial.print(LeftCount);
    Serial.print("   Capacitance Reading: ");
    Serial.print(qt_1.measure() - baseline1);
    Serial.print(", ");
    while ((qt_1.measure() - baseline1) > sensitivity) {
    }
    logdata();
  }

  //If right sipper is triggered
  if ((qt_2.measure() - baseline2) > sensitivity) {
    unsigned long start = millis();
    RightCount++;
    digitalWrite(RED_LED, HIGH);
    delay(50);
    digitalWrite(RED_LED, LOW);
    Serial.print("Right: ");
    Serial.print(RightCount);
    Serial.print("   Capacitance Reading: ");
    Serial.print(qt_2.measure() - baseline2);
    Serial.print(", ");
    while ((qt_2.measure() - baseline2) > sensitivity) {
    }
    logdata();
  }
}

void GoToSleep() {
  //LowPower.sleep(10);  //work out actual sleep and interrupts later
  wakecounter++;

  if ((wakecounter % 500) == 0) {
    Adafruit_FreeTouch qt_1 = Adafruit_FreeTouch(A1, OVERSAMPLE_64, RESISTOR_20K, FREQ_MODE_NONE);
    Adafruit_FreeTouch qt_2 = Adafruit_FreeTouch(A2, OVERSAMPLE_64, RESISTOR_20K, FREQ_MODE_NONE);
    zerosensors();
  }
}
