#include <EEPROM.h>
#include "SparkFunBME280.h"
#include "Wire.h"
#include "SPI.h"

// MANUAL SETUP
#define EEPROM_SIZE 1024  // EEPROM Max Size.
#define INDICATOR_LED 13  // Main LED Indicator.
#define DEBUG_SWITCH 4    // Toggle Switch Digital Pin.
#define SAMPLING 1       // Samples Per Second.

// DON'T TOUCH IN THE CODE BELOW
BME280 barometer;

float altReference = -900;
uint32_t entriesCounter = 0;
int debugMode = 0;

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
    float f = 0.00f; 
    EEPROM.get(i*sizeof(float), f);
    serialFloatPrint(f);
  }
}

void updateEntriesCount(uint32_t count) {
  EEPROM.put(EEPROM_SIZE-4, count);
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
        serialIntPrint((uint16_t)(entriesCounter*8));
      break;
      case '4':
        serialIntPrint((uint16_t)SAMPLING);
      break;
      case '5':
        debugMode = 0;
      break;
      case '9':
        updateEntriesCount(0);
        EEPROM.get(EEPROM_SIZE-4, entriesCounter);
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

  pinMode(DEBUG_SWITCH, INPUT);
  pinMode(INDICATOR_LED, OUTPUT);
  
  EEPROM.get(EEPROM_SIZE-4, entriesCounter);

  if(digitalRead(DEBUG_SWITCH) == LOW) {
    digitalWrite(INDICATOR_LED, LOW); // Debug mode ON.
    Serial.println("=========== DEBUG  MODE ===========");
    Serial.println("Available Commands:");
    Serial.println("1 - Dump Binary Data.");
    Serial.println("2 - Check Connection.");
    Serial.println("3 - Read Memory Size.");
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

    EEPROM.put(entriesCounter*sizeof(float), lastAlt);
    entriesCounter++;

    updateEntriesCount(entriesCounter);

    delay(1000/SAMPLING);
  }
}
