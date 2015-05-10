/*
 * LCDBackpack client code.
 *
 * Based on the original Arduino LiquidCrystal library. As the original LiquidCrystal.cpp had
 * no author comments I'll attribute this to the entire Arduino project. This work is based
 * on the github arduino project, version dated Dec 18, 2013.
 *
 * Stephen Davies
 * May 10, 2015
 */

#include "LCDBackpack.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//    DL = 1; 8-bit interface data
//    N = 0; 1-line display
//    F = 0; 5x8 dot character font
// 3. Display on/off control:
//    D = 0; Display off
//    C = 0; Cursor off
//    B = 0; Blinking off
// 4. Entry mode set:
//    I/D = 1; Increment by 1
//    S = 0; No shift
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// LCDBackpack constructor is called).

LCDBackpack::LCDBackpack(uint8_t i2c_address) : _port(Wire), _i2c_address(i2c_address)
{
    init();
}

LCDBackpack::LCDBackpack(TwoWire& port, uint8_t i2c_address) : _port(port), _i2c_address(i2c_address)
{
    init();
}

void LCDBackpack::init()
{
    _port.begin();
    
    _displayfunction = LCD_1LINE | LCD_5x8DOTS;
    
    // Original code was failing (locking up) due to this line.
    // Probably because TWI uses interrupts and they may be not enabled at this point.
    // User will have to call this later i.e. during setup().
    //begin(16, 1);
}

void LCDBackpack::begin(uint8_t cols, uint8_t lines, uint8_t dotsize)
{
    if (lines > 1) {
        _displayfunction |= LCD_2LINE;
    }
    _numlines = lines;
    
    setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);
    
    // for some 1 line displays you can select a 10 pixel high font
    if ((dotsize != LCD_5x8DOTS) && (lines == 1)) {
        _displayfunction |= LCD_5x10DOTS;
    }
    
    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // according to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. Arduino can turn on way before 4.5V and
    // LCD backpack will need time to initialise the display.
    delay(200);
    
    // finally, set # lines, font size, etc.
    command(LCD_FUNCTIONSET | _displayfunction);

    // turn the display on with no cursor or blinking default
    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    display();
    
    // clear it off
    clear();
    
    // Initialize to default text direction (for romance languages)
    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    
    // set the entry mode
    command(LCD_ENTRYMODESET | _displaymode);
}

void LCDBackpack::setRowOffsets(int row0, int row1, int row2, int row3)
{
    _row_offsets[0] = row0;
    _row_offsets[1] = row1;
    _row_offsets[2] = row2;
    _row_offsets[3] = row3;
}

/********** high level commands, for the user! */
void LCDBackpack::clear()
{
    command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
    delayMicroseconds(2000);  // this command takes a long time!
}

void LCDBackpack::home()
{
    command(LCD_RETURNHOME);  // set cursor position to zero
    delayMicroseconds(2000);  // this command takes a long time!
}

void LCDBackpack::setCursor(uint8_t col, uint8_t row)
{
    const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
    if ( row >= max_lines ) {
        row = max_lines - 1;    // we count rows starting w/0
    }
    if ( row >= _numlines ) {
        row = _numlines - 1;    // we count rows starting w/0
    }
    
    command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}

// Turn the display on/off (quickly)
void LCDBackpack::noDisplay()
{
    _displaycontrol &= ~LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LCDBackpack::display()
{
    _displaycontrol |= LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LCDBackpack::noCursor()
{
    _displaycontrol &= ~LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LCDBackpack::cursor()
{
    _displaycontrol |= LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LCDBackpack::noBlink()
{
    _displaycontrol &= ~LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

void LCDBackpack::blink()
{
    _displaycontrol |= LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LCDBackpack::scrollDisplayLeft(void)
{
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void LCDBackpack::scrollDisplayRight(void)
{
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LCDBackpack::leftToRight(void)
{
    _displaymode |= LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void LCDBackpack::rightToLeft(void)
{
    _displaymode &= ~LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void LCDBackpack::autoscroll(void)
{
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void LCDBackpack::noAutoscroll(void)
{
    _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LCDBackpack::createChar(uint8_t location, uint8_t charmap[])
{
    location &= 0x7; // we only have 8 locations 0-7
    command(LCD_SETCGRAMADDR | (location << 3));
    for (int i=0; i<8; i++) {
        write(charmap[i]);
    }
}

/*********** mid level commands, for sending data/cmds */

inline void LCDBackpack::command(uint8_t value)
{
    send(value, LOW);
}

inline size_t LCDBackpack::write(uint8_t value)
{
    send(value, HIGH);
    return 1; // assume sucess
}

/************ low level data pushing commands **********/

// write either command or data
void LCDBackpack::send(uint8_t value, uint8_t mode)
{
    uint8_t out[2];
    out[0] = (mode & 1) << 6;
    out[1] = value;

    _port.beginTransmission(_i2c_address);
    _port.write(out, 2);
    _port.endTransmission();
}
