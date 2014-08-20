#ifndef DISPLAYDRIVER_H
#define DISPLAYDRIVER_H

#include <Arduino.h>

/*
 * Display driver for a 16x16 led dot "display" using 4 (only red) or 6 (red and green) 74HT595 shift
 * register
 * A file DisplayDriver_cfg.h must be provided (see example)
 * update function should be called cylic
 */
typedef uint16_t displayLine_t;
typedef displayLine_t displayPattern_t[16];

#define DISPLAYDRIVER_CLEARDISPLAY  0xff

class DisplayDriver
{
private:
  displayPattern_t displayRAM;

public:
  DisplayDriver( void );
  void setPattern( displayPattern_t pattern );
  void update( void );
};

#endif



