#include "DisplayDriver.h"

DisplayDriver::DisplayDriver( void )
{
  /* set pin mode for all pins involved */
  pinMode(DISPLAYDRIVER_ROW_DATA, OUTPUT);
  pinMode(DISPLAYDRIVER_ROW_CLK, OUTPUT);
  pinMode(DISPLAYDRIVER_ROW_LATCH, OUTPUT);
  pinMode(DISPLAYDRIVER_RED_DATA, OUTPUT);
  pinMode(DISPLAYDRIVER_COLOR_CLK, OUTPUT);
  pinMode(DISPLAYDRIVER_COLOR_LATCH, OUTPUT);
  pinMode(DISPLAYDRIVER_GREEN_DATA, OUTPUT);

  digitalWrite(DISPLAYDRIVER_GREEN_DATA,DISPLAYDRIVER_CLEARDISPLAY);
  digitalWrite(DISPLAYDRIVER_ROW_LATCH,DISPLAYDRIVER_CLEARDISPLAY);
  digitalWrite(DISPLAYDRIVER_COLOR_LATCH,DISPLAYDRIVER_CLEARDISPLAY);
}

/**
 * Pattern to display with next update command
 **/
void DisplayDriver::setPattern(displayPattern_t pattern)
{
  /* Can't use memcpy because pattern needs to be inverted */
  for (int i=0; i<sizeof(displayPattern_t)/sizeof(displayLine_t); i++)
  {
    displayRAM[i] = ~pattern[i];
  }
}

/**
 * Function similar to shiftOut just that a constant value is shifted out.
 * In addition it directly shifts out 16 bit.
 */
void DisplayDriver::clearLine(byte serialPin, byte clockPin)
{
  digitalWrite(clockPin, LOW);
  digitalWrite(serialPin, DISPLAYDRIVER_CLEARDISPLAY);
  for (byte i=0; i<16; i++)
  {
    digitalWrite(clockPin, HIGH);
    digitalWrite(clockPin, LOW);
  }
}


