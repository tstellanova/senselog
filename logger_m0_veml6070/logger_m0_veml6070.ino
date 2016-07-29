

#include "Adafruit_VEML6070.h"
#include <SPI.h>
#include <SD.h>


const int kErrorLED = 13;
const int kSDChipSelect = 4;
const int kStatusLED = 8;
const int kSDCardDetect = 7;

const int kOversampleTime = 60000; //ms
const int kLoopDelayTime = 1000; //ms
const int kNumOversamples = (kOversampleTime/kLoopDelayTime);
const int kMillisToTimestamp = 60000;

#define FIELD_SEPARATOR "\t"

#define FILE_FLUSH_MS 2000     


File _logFile;
Adafruit_VEML6070 _vemlUV = Adafruit_VEML6070();

#define DSERIAL   Serial

unsigned long _lastFlushTime;
unsigned long _startTime;
uint32_t _numSamplesRead = 0;
uint32_t _vemlBucket = 0;

void flashErrorLED() {
  for (int i = 0; i < 3;  i++) {
      digitalWrite(kErrorLED, HIGH);
      delay(100);
      digitalWrite(kErrorLED, LOW); 
      delay(100);
  }
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
  
  if (!SD.begin(kSDChipSelect)){
      failError(F("Couldn't SD.begin"));
  } 
  _logFile = SD.open("LOGGER.TSV", FILE_WRITE);
  if (!_logFile) {
    failError(F("Couldn't open file"));
  }

}

void clearSampleBuffer() {
      //clear the sample buffer
    _numSamplesRead = 0;
    _vemlBucket = 0;
}

void setupPinMap() {
  pinMode(kSDCardDetect, INPUT);  // SD card detect
  pinMode(kStatusLED, OUTPUT);  // Status LED
  pinMode(kErrorLED, OUTPUT); // Error LED
}


void logMsg(String msg) {
   if (DSERIAL) {
    DSERIAL.println(msg);
  }

  if (_logFile) {
    _logFile.println(msg);
    
    long currentTime = millis();
    if(currentTime >= (_lastFlushTime + FILE_FLUSH_MS)) {
       _logFile.flush();
       _lastFlushTime = currentTime;  
    } 
  }
}



void setup() {

  DSERIAL.begin(9600);
  for (int i = 0; i < 10; i++) {
    if (!DSERIAL) {
      digitalWrite(kStatusLED, HIGH);  // started 
      // wait for serial port to connect. Needed for native USB port only
      delay(300);
      digitalWrite(kStatusLED, LOW);  //  done 
    }
  }
  if (DSERIAL) {
    DSERIAL.println("hello DSERIAL");
  }

  setupPinMap();
  digitalWrite(kErrorLED, LOW);

  _vemlUV.begin(VEML6070_1_T);
  
  openSDLog();
  _startTime = millis();
  _lastFlushTime = millis();

  logMsg("min\tVEML");

  clearSampleBuffer();

}



void loop() {
  digitalWrite(kStatusLED, HIGH);  // started 
  int i;
  uint16_t val;

  val = _vemlUV.readUV();

//  if (0 == val) {
//    clearSampleBuffer();
//    delay(kLoopDelayTime/2);
//    return;
//  }
  
  _vemlBucket += val;  
  _numSamplesRead++;

  if (_numSamplesRead >= kNumOversamples) {
    char outBuf[80];
    
    //average the veml values
    uint32_t vemlReport =  _vemlBucket / _numSamplesRead;
    int timestamp = (millis() - _startTime)/kMillisToTimestamp;

    sprintf(outBuf,"%d\t%d",timestamp,vemlReport);
    logMsg(outBuf);

    clearSampleBuffer();
  }
    
  digitalWrite(kStatusLED, LOW);  //  done 

  delay(kLoopDelayTime);
}


