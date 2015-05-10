/*
 * LCDBackpack client header. LCD commands are passed over I2C to a remote LCDBackpack module.
 *
 * By default, the Wire port is used.
 *
 * Based on the original Arduino LiquidCrystal library. As the original LiquidCrystal.h had
 * no author comments I'll attribute this to the entire Arduino project. This work is based
 * on the github arduino project, version dated Dec 18, 2013.
 *
 * Stephen Davies
 * May 10, 2015
 */

#ifndef LCDBACKPACK_H_
#define LCDBACKPACK_H_

#include <inttypes.h>
#include "Print.h"
#include <Wire.h>

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

class LCDBackpack : public Print {
public:
  /*
   * Regular constructor, note - i2c_address is 7 bit I2C address (see Wire).
   */
  LCDBackpack(uint8_t i2c_address);
   
  /*
   * Constructor intended for use with Arduino Due (has two I2C ports) and user wants
   * to select which port they use i.e. Wire1.
   */
  LCDBackpack(TwoWire& port, uint8_t i2c_address);
    
  void init();
    
  /*
   * User needs to call this at least once e.g. from setup(). This is because interrupts
   * do not get enabled until setup() and calling this from the constructor of an object
   * in global scope results in lockup.
   */
  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);

  void clear();
  void home();

  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();
  void rightToLeft();
  void autoscroll();
  void noAutoscroll();

  void setRowOffsets(int row1, int row2, int row3, int row4);
  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t); 
  virtual size_t write(uint8_t);
  void command(uint8_t);
  
  using Print::write;
private:
  void send(uint8_t, uint8_t);

  TwoWire& _port;
  uint8_t _i2c_address;
    
  uint8_t _displayfunction;
  uint8_t _displaycontrol;
  uint8_t _displaymode;

  uint8_t _numlines;
  uint8_t _row_offsets[4];
};

#endif  // LCDBACKPACK_H_
