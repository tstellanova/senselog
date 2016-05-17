



#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define BMP_MOSI 11 
#define BMP_MISO 12
#define BMP_SCK 13


#define STATUS_LED  5
#define SDCARD_DETECT 6
#define BMP_CS0 2
#define BMP_CS1 3


#define FIELD_SEPARATOR "\t"

#define FILE_FLUSH_MS 2000     

const int SDCARD_CS = SS; //SS;

//Adafruit_BMP280 bme0; // I2C
//Adafruit_BMP280 bme0(BMP_CS0); // hardware SPI
Adafruit_BMP280 bme0(BMP_CS0, BMP_MOSI, BMP_MISO,  BMP_SCK);
Adafruit_BMP280 bme1(BMP_CS1, BMP_MOSI, BMP_MISO,  BMP_SCK);
File _logFile;


unsigned long _lastFlushTime;

void openSDLog() {
  
  if(!digitalRead(SDCARD_DETECT)) {
    if (!SD.begin(SDCARD_CS)){
        Serial.println(F("Couldn't SD.begin"));
        return;
      } 

    _logFile = SD.open("LOG00007.TXT", FILE_WRITE);
    if (!_logFile) {
      Serial.println(F("Couldn't open file"));
      //while(1);
    }

   if(digitalRead(SDCARD_CS) == LOW) {
      Serial.println(F("SDCARD_CS is LOW"));
      digitalWrite(SDCARD_CS, HIGH);
   }
  }
  else {
      Serial.println(F("No SD card detected"));
  }
  
}

void setupPinMap() {
  pinMode(SDCARD_DETECT, INPUT);  // SD card detect
  pinMode(STATUS_LED, OUTPUT);  // Status LED
  
  pinMode(SDCARD_CS, OUTPUT); // SPI chip select for SD card
  digitalWrite(SDCARD_CS, HIGH);
  
  pinMode(BMP_CS0, OUTPUT);
  digitalWrite(BMP_CS0, HIGH);

  pinMode(BMP_CS1, OUTPUT);
  digitalWrite(BMP_CS1, HIGH);
}

bool startOneSensor(Adafruit_BMP280& sensor) {
  int startupCount = 0;
  bool started = false;

  while(!started && (startupCount < 10)) {
    startupCount++;
    started = sensor.begin();
    delay(10);
  }

  return started;
}

void setup() {
  setupPinMap();

  Serial.begin(9600);

  if (!startOneSensor(bme0)) {  
    Serial.println(F("bme0 not responding"));
    while(true);
  }
  if (!startOneSensor(bme1)) {  
    Serial.println(F("bme1 not responding"));
    while(true);
  }

  _lastFlushTime = millis();
   //openSDLog();

}

void loop() {
  float p0, p1, dP;
  digitalWrite(STATUS_LED, HIGH);  // started 

  p0 = bme0.readPressure();
  p1 = bme1.readPressure();
  dP = (p1 - p0);

  Serial.println(dP);

  if (_logFile) {
    _logFile.println(dP);
    long currentTime = millis();
    if(currentTime >= (_lastFlushTime + FILE_FLUSH_MS)) {
       _logFile.flush();
       _lastFlushTime = currentTime;  
    } 
  }

  digitalWrite(STATUS_LED, LOW);  //  done 

  delay(100);
}
