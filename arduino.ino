#include <EEPROM.h>
#define EEPROM_SIZE 1024

void writeNumber(int addr, uint16_t value) {
  byte st = (value & 0xFF);
  byte nd = ((value >> 8) & 0xFF);
  EEPROM.write(addr, st);
  EEPROM.write(addr + 1, nd);
}

uint16_t readNumber(int addr) {
  uint8_t st = EEPROM.read(addr);
  uint8_t nd = EEPROM.read(addr + 1);
  return ((st << 0) & 0xFFFFFF) + ((nd << 8) & 0xFFFFFFFF);
}

void dumpData() {
  for (int i=0; i < EEPROM_SIZE; i+=2) {
    for (unsigned int mask = 0x8000; mask; mask >>= 1) {
      Serial.print(mask&readNumber(i)?'1':'0');
    }
  }
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));
  
  for (int i = 0; i < EEPROM_SIZE; i+=2) {
    writeNumber(i, i);
  }
  
}

void loop() {
  if (Serial.available() > 0) {
    int inByte = Serial.read();
    switch(inByte) {
      case '1':
        dumpData();
      break;
      case '2':
        Serial.print("CONN_OK");
      break;
      case '3':
        Serial.print(EEPROM_SIZE);
      break;
      default:
        Serial.print("NOK");
      break;
    }
    Serial.flush();
  }
}
