///////////////////////////////////////////////////////////////
//Include libraries
///////////////////////////////////////////////////////////////
#include <SPI.h>
#include <SD.h>
#include <Tubetrode.h>

#define buttonPin 10
#define buzzerPin A0

///////////////////////////////////////////////////////////////
// Initialize Tubetrode with sensor addresses and enable pin
///////////////////////////////////////////////////////////////
#define HALL_EN_GPIO 5  // !! User defined, turns on Tubetrode module (on by default)
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
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  tone(buzzerPin, 1000, 1000);
  delay(1000); // block for tone

  ///////////////////////////////////////////////////////////////
  // Start serial communication
  ///////////////////////////////////////////////////////////////
  unsigned long startTime = millis();
  while (!Serial && millis() - startTime < 5000) {
    // Wait for serial port to connect or timeout after 5 seconds
  }

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
    // !! no point to continue
    while (true) {
      digitalWrite(13, ~digitalRead(13));
      delay(100);
    }
  } else {
    Serial.println("Card present and initialized.");
  }

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
  File dataFile = SD.open(filename, FILE_WRITE);                                     // open the file with the name "filename"
  if (dataFile) {                                                                    // if the file is available...
    Serial.println("Millis,vBAT,Mag1,Mag2,Mag3,Mag4,Mag5,Mag6,Mag7,Mag8,EstPos");    // Write dataString to the SD card
    dataFile.println("Millis,vBAT,Mag1,Mag2,Mag3,Mag4,Mag5,Mag6,Mag7,Mag8,EstPos");  // Write dataString to the SD card
    dataFile.close();                                                                // Close the file
  }
}

void loop() {
  // wait for button, delay below acts as debounce
  while (digitalRead(buttonPin)) {}
  tone(buzzerPin, 1000, 500);

  ///////////////////////////////////////////////////////////////
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
  dataString = String(timeStamp) + "," + String(measuredvbat) + "," + String(sensorValues[0], 3) + "," + String(sensorValues[1], 3) + "," + String(sensorValues[2], 3) + "," + String(sensorValues[3], 3) + "," + String(sensorValues[4], 3) + "," + String(sensorValues[5], 3) + "," + String(sensorValues[6], 3) + "," + String(sensorValues[7], 3) + ',' + String(pos, 3);  // put everything together to write to the card

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

  ///////////////////////////////////////////////////////////////
  // Wait for a short period (should improve this to make an actual user settable sampling rate)
  ///////////////////////////////////////////////////////////////
  // delay(9828);
  delay(200);                  // pause/debounce, tone is non-blocking
  tone(buzzerPin, 4000, 100);  // ready
}

void ReadBatteryLevel() {
  measuredvbat = analogRead(A7);
  measuredvbat *= 2;     // we divided by 2, so multiply back
  measuredvbat *= 3.3;   // Multiply by 3.3V, our reference voltage
  measuredvbat /= 1024;  // convert to voltage
}
