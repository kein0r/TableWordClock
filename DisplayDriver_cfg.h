#ifndef DISPLAYDRIVER_CFG_H
#define DISPLAYDRIVER_CFG_H

/* adapt the following to your hardware */
/* ROW */
#define DISPLAYDRIVER_ROW_DATA    5   /* named SERB on board */
#define DISPLAYDRIVER_ROW_CLK     6   /* named SRCLKB on board */
#define DISPLAYDRIVER_ROW_LATCH   7   /* named RCLKB on board */

/* Dot for each row (i.e. column) */
#define DISPLAYDRIVER_GREEN_DATA  8   /* named SERA on board */
#define DISPLAYDRIVER_COLOR_CLK   9   /* named SRCLKA on board */
#define DISPLAYDRIVER_COLOR_LATCH 10  /* named RCLKA on board */
#define DISPLAYDRIVER_RED_DATA    11  /* named SERC on board */

#define DISPLAYDRIVER_SHIFTORDER  LSBFIRST

#define DISPLAYDRIVER_CLEARDISPLAYBEFOREUPDATE

#endif
