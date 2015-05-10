
/*
 * A simple example using the LCDBackpack module.
 *
 * Stephen Davies
 * May 10, 2015
 */

#include <Wire.h>
#include <LCDBackpack.h>

LCDBackpack lcd(0x3A);  // NB. uses 7-bit I2C addressing (same as Wire)

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);
}

