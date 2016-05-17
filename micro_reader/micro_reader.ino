



#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define BMP_MOSI 16
#define BMP_MISO 14
#define BMP_SCK 15

#define STATUS_LED  5
#define BMP_CS0 2
#define BMP_CS1 3


#define FIELD_SEPARATOR "\t"


//Adafruit_BMP280 bme0; // I2C
//Adafruit_BMP280 bme0(BMP_CS0); // hardware SPI
Adafruit_BMP280 bme0(BMP_CS0, BMP_MOSI, BMP_MISO,  BMP_SCK);
Adafruit_BMP280 bme1(BMP_CS1, BMP_MOSI, BMP_MISO,  BMP_SCK);


unsigned long _lastFlushTime;



void setupPinMap() {
  pinMode(STATUS_LED, OUTPUT);  // Status LED

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
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

//  //open UART as well
//  Serial1.begin(9600);
//  while (!Serial1) {
//    ; // wait for serial port to connect. Needed for native USB port only
//  }

  
  if (!startOneSensor(bme0)) {  
    Serial.println(F("bme0 not responding"));
    while(true);
  }
  if (!startOneSensor(bme1)) {  
    Serial.println(F("bme1 not responding"));
    while(true);
  }

}

void loop() {
  float p0, p1, dP;
  digitalWrite(STATUS_LED, HIGH);  // started 

  p0 = bme0.readPressure();
  p1 = bme1.readPressure();
  dP = (p1 - p0);

  Serial.println(dP);


  digitalWrite(STATUS_LED, LOW);  //  done 

  delay(100);
}
