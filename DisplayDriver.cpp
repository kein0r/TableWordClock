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
void DisplayDriver::setPattern(uint16_t pattern[])
{
  memcpy( displayRAM, pattern, sizeof(displayRAM) ); 
}

/**
 * Prints the data in displayRAM on the display
 * For row selecting a one is shifted inside 74HTC959 with each for-loop
 * by just raising clock once every cycle. This way much less data must be
 * shifted out
*/
void DisplayDriver::update(void)
{
  /* put a HIGH on row clk pin which will be shifted later */
  digitalWrite(DISPLAYDRIVER_ROW_DATA, HIGH);
  for (uint8_t rowCounter = 0; rowCounter < 16; rowCounter++)
  {
    /* set clock and latch pin low now to make it stay like this for some time */
    digitalWrite(DISPLAYDRIVER_ROW_LATCH, LOW);
    digitalWrite(DISPLAYDRIVER_ROW_CLK, LOW);

    /* first clear display to avoid ghost effects */
    digitalWrite(DISPLAYDRIVER_COLOR_LATCH,LOW);
    shiftOut(DISPLAYDRIVER_RED_DATA, DISPLAYDRIVER_COLOR_CLK, DISPLAYDRIVER_SHIFTORDER, DISPLAYDRIVER_CLEARDISPLAY);
    shiftOut(DISPLAYDRIVER_RED_DATA, DISPLAYDRIVER_COLOR_CLK, DISPLAYDRIVER_SHIFTORDER, DISPLAYDRIVER_CLEARDISPLAY);
    digitalWrite(DISPLAYDRIVER_COLOR_LATCH,HIGH);

    /* select row */
    /* generate clock pulse to shift row by one */
    digitalWrite(DISPLAYDRIVER_ROW_CLK, HIGH);
    digitalWrite(DISPLAYDRIVER_ROW_LATCH, HIGH);
    /* remove high on row clock pin */
    digitalWrite(DISPLAYDRIVER_ROW_DATA, LOW);

    /* now output data for this row */
    digitalWrite(DISPLAYDRIVER_COLOR_LATCH,LOW);
    shiftOut(DISPLAYDRIVER_RED_DATA, DISPLAYDRIVER_COLOR_CLK, DISPLAYDRIVER_SHIFTORDER, displayRAM[rowCounter] >> 8 );
    shiftOut(DISPLAYDRIVER_RED_DATA, DISPLAYDRIVER_COLOR_CLK, DISPLAYDRIVER_SHIFTORDER, displayRAM[rowCounter] );
    digitalWrite(DISPLAYDRIVER_COLOR_LATCH,HIGH);
  }
}


