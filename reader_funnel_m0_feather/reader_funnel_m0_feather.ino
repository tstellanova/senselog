/**
 * 
 * 
 */

#include <OpenBMP280.h>
` #include <SD.h>

// base SPI pins
#define BMP_MOSI 13
#define BMP_MISO 12
#define BMP_SCK 11

// SPI chip select pins
#define BMP_CS0 20
#define BMP_CS1 21
#define BMP_CS2 5
#define BMP_CS3 6
#define BMP_CS4 9
#define BMP_CS5 10

const int kErrorLED = 13;

// SD card constants
const int kSDChipSelect = 4;
const int kSDStatusLED = 8;
const int kSDCardDetect = 7;
const int kSDFileFlush_ms = 2000;

// main user serial output port
#define USERIAL Serial1

// debug serial output port
#define DSERIAL Serial

#define FIELD_SEPARATOR "\t"

// time at which we start this app
unsigned long _startTime; 

// sensors
const int kNumSensors = 6;
const int _select_pins[kNumSensors] = { BMP_CS0, BMP_CS1, BMP_CS2, BMP_CS3, BMP_CS4, BMP_CS5 };
OpenBMP280* _sensor[kNumSensors];

// logging
File _logFile;
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
    DSERIAL.println(msg);
    DSERIAL.println(msg);
    DSERIAL.println(msg);
    DSERIAL.flush();
    while(true) {
      flashErrorLED();
    }
}

void openSDLog() {
  //determine whether SD card is present
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
  
  _lastFlushTime = _startTime;
}

void setupPinMap() {
  //config SD pins
  pinMode(kSDCardDetect, INPUT);  // SD card presence detector
  pinMode(kSDStatusLED, OUTPUT);  // Status LED

  //config base SPI pins
  pinMode(BMP_SCK, OUTPUT);
  pinMode(BMP_MOSI, OUTPUT);
  pinMode(BMP_MISO, INPUT);

  //setup all the SPI sensor select pins
  for (int i = 0; i  < kNumSensors ; i++) {
    pinMode(_select_pins[i], OUTPUT);
    digitalWrite(_select_pins[i], HIGH);
  }
}

// Start a single BMP sensor
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

void setupSerialPorts() {
  DSERIAL.begin(9600);
  for (int i = 0; i < 10; i++) {
    if (!DSERIAL) {
      digitalWrite(kErrorLED, HIGH); 
      // wait for serial port to connect. Needed for native USB port only
      delay(100);
      digitalWrite(kErrorLED, LOW);
    }
  }
  if (DSERIAL) {
    DSERIAL.println("hello DSERIAL");
  }
  
  USERIAL.begin(9600);
  for (int i = 0; i < 10; i++) {
    if (!USERIAL) {
      digitalWrite(kErrorLED, HIGH); 
      // wait for serial port to connect.
      delay(100);
      digitalWrite(kErrorLED, LOW);
    }
  }
  if (USERIAL) {
    USERIAL.println("hello USERIAL");
  }
}

void setup() {
  _startTime = millis();

  setupSerialPorts();
  setupPinMap();

  for (int i = 0; i  < kNumSensors ; i++) {
    const int select = _select_pins[i];
    _sensor[i] = new OpenBMP280(select, BMP_MOSI, BMP_MISO, BMP_SCK);
    if (!startOneSensor(*_sensor[i])) {  
      char errBuf[40];
      sprintf(errBuf, "Sensor %d is not responding",i);
      failError(errBuf);
    }

    pinMode(_select_pins[i], OUTPUT);
    digitalWrite(_select_pins[i], HIGH);
  }
 
  openSDLog();
}

void outputVector(float* points)
{
  char outBuf[80];

  long currentTime = millis();
  long dTime = currentTime - _startTime;
  
  sprintf(outBuf,"%8d\t%7.2f\t%7.2f\t%7.2f",dTime,points[0],points[1],points[2]);
  if (DSERIAL) {
    DSERIAL.println(outBuf);
  }

  if (USERIAL) {
    USERIAL.println(outBuf);
  }
  
  if (_logFile) {
    _logFile.println(outBuf);
    
    if(currentTime >= (_lastFlushTime + kSDFileFlush_ms)) {
       _logFile.flush();
       _lastFlushTime = currentTime;  
    } 
  }

}

void loop() {
  float points[3] = {};
  digitalWrite(kSDStatusLED, HIGH);  // started read

  for (int i = 0, j = 0; i  < kNumSensors ; i+=2, j++) {
    float p0 = _sensor[i]->readPressure();
    float p1 = _sensor[i+1]->readPressure();
    points[j] = p0 - p1;
  }
  
  outputVector(points);

  digitalWrite(kSDStatusLED, LOW);  //  done 
  
  delay(10);
}


