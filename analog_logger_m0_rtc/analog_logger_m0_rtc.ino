

#include "RTClib.h"
#include "Adafruit_VEML6070.h"
#include <SPI.h>
#include <SD.h>


const int kErrorLED = 13;
const int kSDChipSelect = 10;
//On the "M0 basic proto" or "Adalogger RTC wing"
//there is no separate status LED
const int kStatusLED = 13;
const int kSDCardDetect = 7;

const int kOversampleTime = 60000; //ms
const int kLoopDelayTime = 1000; //ms
const int kNumOversamples = (kOversampleTime/kLoopDelayTime);

const int kNumSensors = 6;
const int _sensePins[kNumSensors] = {A0, A1, A2, A3, A4, A5};

#define FIELD_SEPARATOR "\t"

#define FILE_FLUSH_MS 2000     


File _logFile;
RTC_PCF8523 _rtc;
char _dateTimeBuf[17];  
Adafruit_VEML6070 _vemlUV = Adafruit_VEML6070();

#define DSERIAL   Serial

unsigned long _lastFlushTime;
unsigned long _startTime;
uint32_t _numSamplesRead = 0;
uint32_t _senseBucket[kNumSensors] = {};
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
    for (int i = 0 ; i < kNumSensors; i++) {
      _senseBucket[i] = 0;
    }
    _vemlBucket = 0;
}

void setupPinMap() {
  pinMode(kSDCardDetect, INPUT);  // SD card detect
  pinMode(kStatusLED, OUTPUT);  // Status LED

  for (int i = 0; i < kNumSensors; i++) {
    pinMode(_sensePins[i], INPUT);
  }

  //increase ADC resolution to maximum for the M0 
  analogReadResolution(12);
  
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

// call back for file timestamps
void dateTime(uint16_t* date, uint16_t* time) {
  DateTime now = _rtc.now();

  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(now.year(), now.month(), now.day());

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
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

  //SdFile uses this callback to mark the log file with correct time/date stamp
  SdFile::dateTimeCallback(dateTime);
  

  if (! _rtc.begin()) {
    failError(F("Couldn't start RTC"));
  }
  else if (!_rtc.initialized()) {
    logMsg("Setting RTC timestamp");
    // following line sets the RTC to the date & time this sketch was compiled
    _rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  _vemlUV.begin(VEML6070_1_T);

  
  openSDLog();
  _startTime = millis();
  _lastFlushTime = millis();

  logMsg("             time\tA0\tA1\tA2\tA5\tVEML");

  clearSampleBuffer();

}



void loop() {
  digitalWrite(kStatusLED, HIGH);  // started 
  int i;
  uint16_t val;

  val = _vemlUV.readUV();

  if (0 == val) {
    delay(kLoopDelayTime/2);
    return;
  }
  
  _vemlBucket += val;
  
  for ( i = 0; i < kNumSensors; ++i) {
    val = analogRead(_sensePins[i]);
    _senseBucket[i] += val;
  }

  
  _numSamplesRead++;

  if (_numSamplesRead >= kNumOversamples) {
    char outBuf[80];

    DateTime now = _rtc.now();
    uint32_t timestamp = now.unixtime();
    
    //take the average of the samples in each bucket
    for (i = 0 ; i < kNumSensors; ++i) {
      _senseBucket[i] = _senseBucket[i] / _numSamplesRead;
    }
    //average the veml values
    uint32_t vemlReport =  _vemlBucket / _numSamplesRead;
    
    // format the date output as YYYY-MM-DD HH:MM:SS 
    // eg: 2014-08-19 12:41:35.220
    sprintf(_dateTimeBuf,
    "%4.2d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", 
    now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second());

    sprintf(outBuf,"%s\t%d\t%d\t%d\t%d\t%d",_dateTimeBuf,
    _senseBucket[0],_senseBucket[1],_senseBucket[2],_senseBucket[5],vemlReport);
    logMsg(outBuf);

    clearSampleBuffer();
    
  }
    
  digitalWrite(kStatusLED, LOW);  //  done 

  delay(kLoopDelayTime);
}


