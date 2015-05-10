/*
 * Configures an Arduino as an I2C LCDBackpack module.
 * 
 * The I2C wire protocol is based on the Philips PCF2116 chip.
 * In simple terms it uses the common Hitachi 44780 commands over
 * the wire. Embedded control bytes are used to indicate RS bit
 * settings. Whilst the PCF2116 also embeds a value for the R//W bit,
 * it isn't used here because the Arduino LCD code doesn't require
 * reads (i.e. it is write only).
 *
 * Although the PCF2116 supports multiple commands and data bytes per
 * I2C frame this isn't implemented here, the Arduino LCD client made
 * it tricky to merge multiple commands, each command or data byte
 * is transacted in its own I2C frame.
 *
 * Stephen Davies
 * May 10, 2015
 */
 
#include <Wire.h>
#include <LiquidCrystal.h>

// Multiple PCF2116 control bytes can be embedded in a single I2C frame, see datasheet for details.
#define IS_SINGLE_CONTROL(b) (b & 0x80)

// TRUE if RS == 1
#define IS_DR_SELECTED (control & 0x40)

// State machine for processing a single I2C frame
enum ReceiveState { CONTROL, DATA_1, DATA_N };

// current, i.e. last, control byte received
uint8_t control;

// Set up for non-standard LCD (Iteadstudio LCD1602)
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

void setup() {
  Wire.begin(0x3A);
  Wire.onReceive(wireHandler);
}

void loop() {
}

void wireHandler(int numBytes)
{
  ReceiveState state = CONTROL;
  
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
  // Special handling for function set command,
  // don't want it changing the configured interface bits.
  if ((b & 0xE0) == LCD_FUNCTIONSET) {
    b &= 0xEF; // clear data length bit i.e 4/8 bit interface
  }
  
  lcd.command(b);
}

void writeDataReg(uint8_t b)
{
  lcd.write(b);
}

