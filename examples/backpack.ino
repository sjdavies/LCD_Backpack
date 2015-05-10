#include <Wire.h>
#include <LiquidCrystal.h>

#define IS_SINGLE_CONTROL(b) (b & 0x80)
#define IS_DR_SELECTED (control & 0x40)

enum ReceiveState { CONTROL, DATA_1, DATA_N };

uint8_t control;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup() {
  Wire.begin(0x3A);
  Wire.onReceive(wireHandler);
  Serial.begin(115200);
  Serial.println("ready");
}

void loop() {
}

void wireHandler(int numBytes)
{
  ReceiveState state = CONTROL;
  
  Serial.println(numBytes);
  
  for(int i = 0; i < numBytes; i++) {
    uint8_t value = Wire.read();
    
    switch(state) {
      case CONTROL:
        control = value & 0x60;
        if (IS_SINGLE_CONTROL(value)) {
          state = DATA_1;
        } else {
          state = DATA_N;
        }
        break;
      case DATA_1:
        state = CONTROL;
        // fallthru!
      case DATA_N:
        if (IS_DR_SELECTED) {
          writeDataReg(value);
        } else {
          writeCommandReg(value);
        }
        break;
    }
  }
}

void writeCommandReg(uint8_t b)
{
  if ((b & 0xE0) == LCD_FUNCTIONSET) {
    b &= 0xEF; // clear data length bit i.e 4/8 bit interface
  }
  
  lcd.command(b);
}

void writeDataReg(uint8_t b)
{
  lcd.write(b);
}

