#include <Adafruit_BMP280.h>

/*
  SD card datalogger

 This example shows how to log data from three analog sensors
 to an SD card using the SD library.

 The circuit:
 * analog sensors on analog ins 0, 1, and 2
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4

 created  24 Nov 2010
 modified 9 Apr 2012
 by Tom Igoe

 This example code is in the public domain.

 */

#include <SPI.h>
#include <SD.h>

const int kSDChipSelect = 4;
const int kSDStatusLED = 8;
const int kSDCardDetect = 7;
const int kGenericStatusLED = 13;

//#4 - used as the MicroSD card CS (chip select) pin
//#7 - used as the MicroSD card CD (card detect) pin. If you want to detect when a card is inserted/removed, 
//configure this pin as an input with a pullup. When the pin reads low (0V) then there is no card inserted. When the pin reads high, then a card is in place. 
//#8 - This pin was also left over, so we tied it to a green LED, its next to the SD card. 

File _dataFile;

void setupPinMap() {
  pinMode(kSDStatusLED, OUTPUT);
  pinMode(kGenericLED, OUTPUT);
  pinMode(kSDCardDetect, INPUT); 

}

void initLogFile() {

  if (digitalRead(kSDCardDetect)) {
    _dataFile = SD.open("datalog.txt", FILE_WRITE);
  
    Serial.println(F("Initializing SD card..."));
  
    // see if the card is present and can be initialized:
    if (!SD.begin(kSDChipSelect)) {
      Serial.println(F("Card failed, or not present"));
      // don't do anything more:
      return;
    }
    else {
      _dataFile = SD.open("datalog.txt", FILE_WRITE);
      if (_dataFile) { 
        Serial.println(F("dataFile initialized."));
      }
    }
  
  }
  else {
      Serial.println(F("No SD Card detected"));
  }
}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  setupPinMap();
  initLogFile();

}

void loop() {
  // make a string for assembling the data to log:
  String dataString = "";

  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 3; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 2) {
      dataString += ",";
    }
  }

  digitalWrite(kSDStatusLED, HIGH);

  // if the file is available, write to it:
  if (_dataFile) {
    _dataFile.println(dataString);
    // print to the serial port too:
    Serial.println(dataString);
  }

  digitalWrite(kSDStatusLED, LOW);

  delay(100);
}









