#include "DisplayDriver.h"
#include "DisplayDriver_cfg.h"

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

  digitalWrite(DISPLAYDRIVER_GREEN_DATA,LOW);
  digitalWrite(DISPLAYDRIVER_ROW_LATCH,LOW);
  digitalWrite(DISPLAYDRIVER_COLOR_LATCH,LOW);
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
 * Prints the data in displayRAM on the display
 * Since always two rows are used together (i.e. horizontal resultion is halfed) these two rows will
 * be activated together. Therefore two logical one are shifted in at first.
 * After each line is sent the rows are shifted by two again.
*/
void DisplayDriver::update(void)
{
  /* put a HIGH on row clk pin which will be shifted later */
  digitalWrite(DISPLAYDRIVER_ROW_DATA, HIGH);
  
  for (uint8_t rowCounter = 0; rowCounter < 16; rowCounter += 2)
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


