#ifndef DISPLAYDRIVER_H
#define DISPLAYDRIVER_H

#include <Arduino.h>

/*
 * Display driver for a 16x16 led dot "disaply" using 4 (only red) or 6 (red and green) 74HT595 shift
 * register
 * A file DisplayDriver_cfg.h must be provided (see example)
 * update function should be called cylic
 */

class DisplayDriver
{
private:
  uint16_t displayRAM[16];

public:
  DisplayDriver( void );
  void setPattern( uint16_t pattern[] );
  void update( void );
};

#endif


