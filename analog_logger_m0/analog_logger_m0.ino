

#include <SPI.h>
#include <SD.h>


const int kErrorLED = 13;
const int kSDChipSelect = 4;
const int kSDStatusLED = 8;
const int kSDCardDetect = 7;

const int kOversampleTime = 60000; //ms
const int kLoopDelayTime = 500; //ms
const int kNumOversamples = (kOversampleTime/kLoopDelayTime);

const int kNumSensors = 4;
const int _sensePins[kNumSensors] = {A0, A1, A2, A3};

#define FIELD_SEPARATOR "\t"

#define FILE_FLUSH_MS 2000     


File _logFile;

#define DSERIAL   Serial

unsigned long _lastFlushTime;
unsigned long _startTime;
uint32_t _numSamplesRead;
uint32_t _senseBucket[kNumSensors];

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
  
  // if(digitalRead(kSDCardDetect)) {
    if (!SD.begin(kSDChipSelect)){
        failError(F("Couldn't SD.begin"));
      } 

    _logFile = SD.open("LOGGER.TSV", FILE_WRITE);
    if (!_logFile) {
      failError(F("Couldn't open file"));
    }
 // }
 // else {
 //     failError(F("No SD card detected"));
 // }
  
}

void clearSampleBuffer() {
      //clear the sample buffer
    _numSamplesRead = 0;
    for (int i = 0 ; i < kNumSensors; i++) {
      _senseBucket[i] = 0;
    }
}

void setupPinMap() {
  pinMode(kSDCardDetect, INPUT);  // SD card detect
  pinMode(kSDStatusLED, OUTPUT);  // Status LED


  for (int i = 0; i < kNumSensors; i++) {
    pinMode(_sensePins[i], INPUT);
  }
  
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
  for (int i = 0; i < 5; i++) {
    if (!DSERIAL) {
      digitalWrite(kSDStatusLED, HIGH);  // started 
      // wait for serial port to connect. Needed for native USB port only
      delay(500);
      digitalWrite(kSDStatusLED, LOW);  //  done 
    }
  }
  if (DSERIAL) {
    DSERIAL.println("hello DSERIAL");
  }

  setupPinMap();
  clearSampleBuffer();

  openSDLog();
  _startTime = millis();
  _lastFlushTime = millis();

  logMsg("secs\tA0\tA1\tA2\tA3");
  _numSamplesRead = 0;
}

void loop() {
  char outBuf[80];
  digitalWrite(kSDStatusLED, HIGH);  // started 

  for (int i = 0; i < kNumSensors; i++) {
    uint16_t val = analogRead(_sensePins[i]);
    _senseBucket[i] += val;
  }
  _numSamplesRead++;

  if (_numSamplesRead >= kNumOversamples) {
    int timestamp = (millis() - _startTime)/1000;

    //take the average of the samples in each bucket
    for (int i = 0 ; i < kNumSensors; i++) {
      _senseBucket[i] /= _numSamplesRead;
    }
    
    sprintf(outBuf,"%d\t%d\t%d\t%d\t%d",timestamp,
    _senseBucket[0],_senseBucket[1],_senseBucket[2],_senseBucket[3]);
    logMsg(outBuf);

    clearSampleBuffer();
    
  }
    
  digitalWrite(kSDStatusLED, LOW);  //  done 

  delay(kLoopDelayTime);
}


