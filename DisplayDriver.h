#ifndef DISPLAYDRIVER_H
#define DISPLAYDRIVER_H

#include <Arduino.h>
#include "DisplayDriver_cfg.h"

/*
 * Display driver for a 16x16 led dot "display" using 4 (only red) or 6 (red and green) 74HT595 shift
 * register
 * A file DisplayDriver_cfg.h must be provided (see example)
 * update function should be called cylic
 */
typedef uint16_t displayLine_t;
typedef displayLine_t displayPattern_t[8];

#define DISPLAYDRIVER_CLEARDISPLAY  0xffff

class DisplayDriver
{
private:
  displayPattern_t displayRAM;
  void clearLine(byte serialPin, byte clockPin); 

public:
  DisplayDriver( void );
  void setPattern( displayPattern_t pattern );
  inline void update( void );
};

/**
 * Prints the data in displayRAM on the display
 * Since always two rows are used together (i.e. horizontal resultion is halfed) these two rows will
 * be activated together. Therefore two logical one are shifted in at first.
 * After each line is sent the rows are shifted by two again.
*/
inline void DisplayDriver::update(void)
{
  /* put a HIGH on row clk pin which will be shifted later */
  digitalWrite(DISPLAYDRIVER_ROW_DATA, HIGH);
  
  for (uint8_t rowCounter = 0; rowCounter < 8; rowCounter++)
  {
    /* set latch pin low before any new action */
    digitalWrite(DISPLAYDRIVER_ROW_LATCH, LOW);

#ifdef DISPLAYDRIVER_CLEARDISPLAYBEFOREUPDATE
    /* first clear display to avoid ghost effects */
    digitalWrite(DISPLAYDRIVER_COLOR_LATCH,LOW);
    clearLine(DISPLAYDRIVER_RED_DATA, DISPLAYDRIVER_COLOR_CLK);
    digitalWrite(DISPLAYDRIVER_COLOR_LATCH,HIGH);
#endif

    /* select row */
    /* generate two clock pulse to shift row by two (i.e. the two logical one) */
    digitalWrite(DISPLAYDRIVER_ROW_CLK, LOW);
    digitalWrite(DISPLAYDRIVER_ROW_CLK, HIGH);
    digitalWrite(DISPLAYDRIVER_ROW_CLK, LOW);
    digitalWrite(DISPLAYDRIVER_ROW_CLK, HIGH);
    /* enable ROW */
    digitalWrite(DISPLAYDRIVER_ROW_LATCH, HIGH);
    /* set data pin low again (actually only needed in first run) */
    digitalWrite(DISPLAYDRIVER_ROW_DATA, LOW);

    /* now output data for the selected rows */
    if (displayRAM[rowCounter] != DISPLAYDRIVER_CLEARDISPLAY)
    {
      digitalWrite(DISPLAYDRIVER_COLOR_LATCH,LOW);
      shiftOut(DISPLAYDRIVER_RED_DATA, DISPLAYDRIVER_COLOR_CLK, DISPLAYDRIVER_SHIFTORDER, displayRAM[rowCounter] );
      shiftOut(DISPLAYDRIVER_RED_DATA, DISPLAYDRIVER_COLOR_CLK, DISPLAYDRIVER_SHIFTORDER, displayRAM[rowCounter] >> 8 );
      digitalWrite(DISPLAYDRIVER_COLOR_LATCH,HIGH);
      //__asm__("nop\n\t""nop\n\t""nop\n\t""nop\n\t");
    };
  }
#ifdef DISPLAYDRIVER_CLEARDISPLAYBEFOREUPDATE
    /* first clear display to avoid ghost effects */
    digitalWrite(DISPLAYDRIVER_COLOR_LATCH,LOW);
    clearLine(DISPLAYDRIVER_RED_DATA, DISPLAYDRIVER_COLOR_CLK);
    digitalWrite(DISPLAYDRIVER_COLOR_LATCH,HIGH);
#endif
}

#endif



