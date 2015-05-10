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
    
    begin(16, 1);
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
    // before sending commands. Arduino can turn on way before 4.5V so we'll wait 50
    delayMicroseconds(50000);
    
    //put the LCD into 4 bit or 8 bit mode
//    if (! (_displayfunction & LCD_8BITMODE)) {
//        // this is according to the hitachi HD44780 datasheet
//        // figure 24, pg 46
//        
//        // we start in 8bit mode, try to set 4 bit mode
//        write4bits(0x03);
//        delayMicroseconds(4500); // wait min 4.1ms
//        
//        // second try
//        write4bits(0x03);
//        delayMicroseconds(4500); // wait min 4.1ms
//        
//        // third go!
//        write4bits(0x03);
//        delayMicroseconds(150);
//        
//        // finally, set to 4-bit interface
//        write4bits(0x02);
//    } else {
//        // this is according to the hitachi HD44780 datasheet
//        // page 45 figure 23
//        
//        // Send function set command sequence
//        command(LCD_FUNCTIONSET | _displayfunction);
//        delayMicroseconds(4500);  // wait more than 4.1ms
//        
//        // second try
//        command(LCD_FUNCTIONSET | _displayfunction);
//        delayMicroseconds(150);
//        
//        // third go
//        command(LCD_FUNCTIONSET | _displayfunction);
//    }
    
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
    uint8_t control = (mode & 1) << 6;
    
    _port.beginTransmission(_i2c_address);
    _port.write(control);
    _port.write(value);
    _port.endTransmission();
}
