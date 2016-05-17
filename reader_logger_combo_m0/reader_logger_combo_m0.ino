#include <OpenBMP280.h>


#include <SPI.h>
#include <SD.h>


#define BMP_MOSI 13
#define BMP_MISO 12
#define BMP_SCK 11

#define BMP_CS0 20
#define BMP_CS1 21
#define BMP_CS2 5
#define BMP_CS3 6
#define BMP_CS4 9
#define BMP_CS5 10

const int kErrorLED = 13;
const int kSDChipSelect = 4;
const int kSDStatusLED = 8;
const int kSDCardDetect = 7;


#define FIELD_SEPARATOR "\t"

#define FILE_FLUSH_MS 2000     

const int SDCARD_CS = SS; //SS;

//Adafruit_BMP280 bme0; // I2C
//Adafruit_BMP280 bme0(BMP_CS0); // hardware SPI
OpenBMP280 bme0(BMP_CS0, BMP_MOSI, BMP_MISO,  BMP_SCK);
OpenBMP280 bme1(BMP_CS1, BMP_MOSI, BMP_MISO,  BMP_SCK);
OpenBMP280 bme2(BMP_CS2, BMP_MOSI, BMP_MISO,  BMP_SCK);
OpenBMP280 bme3(BMP_CS3, BMP_MOSI, BMP_MISO,  BMP_SCK);
OpenBMP280 bme4(BMP_CS4, BMP_MOSI, BMP_MISO,  BMP_SCK);
OpenBMP280 bme5(BMP_CS5, BMP_MOSI, BMP_MISO,  BMP_SCK);

File _logFile;

#define DSERIAL   Serial
#define USERIAL   Serial1

unsigned long _lastFlushTime;

void flashErrorLED() {
      digitalWrite(kErrorLED, HIGH);
      delay(100);
      digitalWrite(kErrorLED, LOW); 
      delay(100);
      digitalWrite(kErrorLED, HIGH);
      delay(100);
      digitalWrite(kErrorLED, LOW);
      delay(100);
      digitalWrite(kErrorLED, HIGH);
      delay(100);
      digitalWrite(kErrorLED, LOW);
      delay(750);
}
void failError(String msg) {
    if (DSERIAL) {
      DSERIAL.println(msg);
      DSERIAL.println(msg);
      DSERIAL.println(msg);
      DSERIAL.flush();
    }

    while(true) {
      flashErrorLED();
    }
}

void openSDLog() {
  
  if(digitalRead(kSDCardDetect)) {
    if (!SD.begin(kSDChipSelect)){
        failError(F("Couldn't SD.begin"));
      } 

    _logFile = SD.open("LOGGER.TXT", FILE_WRITE);
    if (!_logFile) {
      failError(F("Couldn't open file"));
    }
  }
  else {
      failError(F("No SD card detected"));
  }
  
}

void setupPinMap() {
  pinMode(kSDCardDetect, INPUT);  // SD card detect
  pinMode(kSDStatusLED, OUTPUT);  // Status LED

  pinMode(BMP_SCK, OUTPUT);
  pinMode(BMP_MOSI, OUTPUT);
  pinMode(BMP_MISO, INPUT);
  
  pinMode(BMP_CS0, OUTPUT);
  digitalWrite(BMP_CS0, HIGH);
  pinMode(BMP_CS1, OUTPUT);
  digitalWrite(BMP_CS1, HIGH);
  pinMode(BMP_CS2, OUTPUT);
  digitalWrite(BMP_CS2, HIGH);
  pinMode(BMP_CS3, OUTPUT);
  digitalWrite(BMP_CS3, HIGH);
  pinMode(BMP_CS4, OUTPUT);
  digitalWrite(BMP_CS4, HIGH);
  pinMode(BMP_CS5, OUTPUT);
  digitalWrite(BMP_CS5, HIGH);
  
}


bool startOneSensor(OpenBMP280& sensor) {
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
  
  USERIAL.begin(9600);
  for (int i = 0; i < 10; i++) {
    if (!USERIAL) {
      digitalWrite(kSDStatusLED, HIGH);  // started 
      // wait for serial port to connect.
      delay(100);
      digitalWrite(kSDStatusLED, LOW);  //  done 
    }
  }
  if (USERIAL) {
    USERIAL.println("hello USERIAL");
  }

  DSERIAL.begin(9600);
  for (int i = 0; i < 10; i++) {
    if (!DSERIAL) {
      digitalWrite(kSDStatusLED, HIGH);  // started 
      // wait for serial port to connect. Needed for native USB port only
      delay(100);
      digitalWrite(kSDStatusLED, LOW);  //  done 
    }
  }
  if (DSERIAL) {
    DSERIAL.println("hello DSERIAL");
  }

  
  setupPinMap();

  _lastFlushTime = millis();

  if (!startOneSensor(bme0)) {  
    failError(F("bme0 not responding"));
  }
  if (!startOneSensor(bme1)) {  
    failError(F("bme1 not responding"));
  }
  if (!startOneSensor(bme2)) {  
    failError(F("bme2 not responding"));
  }
  if (!startOneSensor(bme3)) {  
    failError(F("bme3 not responding"));
  }
  if (!startOneSensor(bme4)) {  
    failError(F("bme4 not responding"));
  }
  if (!startOneSensor(bme5)) {  
    failError(F("bme5 not responding"));
  }

  openSDLog();

}

void loop() {
  char outBuf[80];
  float p0, p1, p2, p3, p4, p5, dp0, dp1, dp2;

  digitalWrite(kSDStatusLED, HIGH);  // started 

  p0 = bme0.readPressure();
  p1 = bme1.readPressure();
  p2 = bme2.readPressure();
  p3 = bme3.readPressure();
  p4 = bme4.readPressure();
  p5 = bme5.readPressure();

  dp0 = (p1 - p0);
  dp1 = (p3 - p2);
  dp2 = (p5 - p4);

  sprintf(outBuf,"%7.2f\t%7.2f\t%7.2f",dp0,dp1,dp2);
  if (DSERIAL) {
    DSERIAL.println(outBuf);
  }

  if (USERIAL) {
    USERIAL.println(outBuf);
  }
  
  if (_logFile) {
    _logFile.println(outBuf);
    
    long currentTime = millis();
    if(currentTime >= (_lastFlushTime + FILE_FLUSH_MS)) {
       _logFile.flush();
       _lastFlushTime = currentTime;  
    } 
  }

  digitalWrite(kSDStatusLED, LOW);  //  done 

  delay(10);
}


