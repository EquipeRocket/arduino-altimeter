#include <EEPROM.h>
#include "SparkFunBME280.h"
#include "Wire.h"
#include "SPI.h"

// MANUAL SETUP
#define INTERNAL_EEPROM_SIZE 1024   // Internal EEPROM Max Size.
#define EXTERNAL_EEPROM_SIZE 64000  // External EEPROM Max Size.
#define EXTERNAL_EEPROM_ADDRS 0x50   // External EEPROM I2C Address.
#define INDICATOR_LED 13            // Main LED Indicator.
#define DEBUG_SWITCH 6              // Toggle Switch Digital Pin.
#define SAMPLING 1                  // Samples Per Second.

// DON'T TOUCH IN THE CODE BELOW
BME280 barometer;

float altReference = -900;
uint32_t entriesCounter = 0;
int debugMode = 0;

void writeToMemory(float value, int addr) {
  uint32_t input = *((uint32_t*)&value);

  Wire.beginTransmission(EXTERNAL_EEPROM_ADDRS);
  Wire.write((int)(addr >> 8));
  Wire.write((int)(addr & 0xFF));
  
  for(int i=0; i<sizeof(input); i++) {
    Wire.write((input >> 8*i) & 0x000000FF);
  }

  Wire.endTransmission();
  delay(5);
}

float readFromMemory(int addr) {
  uint32_t output = 0x00000000;

  Wire.beginTransmission(EXTERNAL_EEPROM_ADDRS);
  Wire.write((int)(addr >> 8));
  Wire.write((int)(addr & 0xFF));
  Wire.endTransmission();

  Wire.requestFrom(EXTERNAL_EEPROM_ADDRS, 4);

  uint8_t buffer[4] = { 0x00, 0x00, 0x00, 0x00 };
  int readBytes = 0;
  
  while(Wire.available()) {
    buffer[readBytes] = Wire.read();
    readBytes++;
  }
  
  for(int i=4; i>=0; i--) {
    output = output << 8;
    output |= buffer[i];
  }

  return *((float*)&output);
}

void serialIntPrint(uint16_t i) {
  for (unsigned int mask = 0x8000; mask; mask >>= 1) {
      Serial.print(mask&i?'1':'0');
    }
}

void serialFloatPrint(float f) {
  byte * b = (byte *) &f;
  for(int i=0; i<4; i++) {
    
    byte b1 = (b[i] >> 4) & 0x0f;
    byte b2 = (b[i] & 0x0f);
    
    char c1 = (b1 < 10) ? ('0' + b1) : 'A' + b1 - 10;
    char c2 = (b2 < 10) ? ('0' + b2) : 'A' + b2 - 10;
    
    Serial.print(c1);
    Serial.print(c2);
  }
}

void uploadMemory() {
  for(uint32_t i=0; i<entriesCounter; i++) {
    serialFloatPrint(readFromMemory(i*sizeof(float)));
  }
}

void updateEntriesCount(uint32_t count) {
  EEPROM.put(0, count); // Internal Memory
}

void listenCommanader() {
  if (Serial.available() > 0) {
    int inByte = Serial.read();
    switch(inByte) {
      case '1':
        uploadMemory();
      break;
      case '2':
        Serial.print("CONN_OK");
      break;
      case '3':
        serialIntPrint((uint16_t)(entriesCounter));
      break;
      case '4':
        serialIntPrint((uint16_t)SAMPLING);
      break;
      case '5':
        debugMode = 0;
      break;
      case '9':
        updateEntriesCount(0);
        EEPROM.get(0, entriesCounter); // Internal Memory
      break;
    }
    Serial.flush();
  } 
}

void setup() {
  Serial.begin(115200);
  Serial.println("Altimeter v1.0 - Equipe Rocket 2017");
  
  barometer.settings.commInterface = I2C_MODE;
  barometer.settings.I2CAddress = 0x76;
  barometer.settings.runMode = 3;
  barometer.settings.tStandby = 0;
  barometer.settings.filter = 4;
  barometer.settings.tempOverSample = 5;
  barometer.settings.pressOverSample = 5;
  barometer.settings.humidOverSample = 1;
  delay(50);
  barometer.begin();
  delay(50);

  Wire.begin();

  pinMode(DEBUG_SWITCH, INPUT_PULLUP);
  pinMode(INDICATOR_LED, OUTPUT);
  
  EEPROM.get(0, entriesCounter); // Internal Memory

  if(digitalRead(DEBUG_SWITCH) == LOW) {
    digitalWrite(INDICATOR_LED, LOW); // Debug mode ON.
    Serial.println("=========== DEBUG  MODE ===========");
    Serial.println("Available Commands:");
    Serial.println("1 - Dump Binary Data.");
    Serial.println("2 - Check Connection.");
    Serial.println("3 - Read Sample Size.");
    Serial.println("4 - Read Sample Rate.");
    Serial.println("5 - Toggle Stnd Mode.");
    Serial.println("9 - Reset Memory Alc.");
    debugMode = 1;
  } else {
    digitalWrite(INDICATOR_LED, HIGH); // Debug mode OFF.
    delay(200);
    debugMode = 0;
  }
}

void loop() {
  if(debugMode) {
    listenCommanader();
  } else {
    barometer.readTempC();

    if(altReference==-900) {
      altReference = barometer.readFloatAltitudeMeters();
      delay(50);
    }
    
    float lastAlt = barometer.readFloatAltitudeMeters() - altReference;

    Serial.print("Current Altitude (m): ");
    Serial.println(lastAlt);

    writeToMemory(lastAlt, entriesCounter*sizeof(float));
    entriesCounter++;

    updateEntriesCount(entriesCounter);

    delay(1000/SAMPLING);
  }
}
