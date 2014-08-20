/*
Toughts:
 Einstellen mittels binärcode dabei leuchtet dian oder fen auf
 */
/*
 * !!!!!!!!
 * This sketch needs the following non-standard libraries (install them in the Arduino library directory):
 * TimerOne: http://www.arduino.cc/playground/Code/Timer1
 * !!!!!!!!

*/
#include <TimerOne.h>
#include "DisplayDriver.h"


DisplayDriver displayDriver;

uint16_t myTestPattern[16] = {
  0xaaaa, 0xaaaa, 0x5555, 0x5555, 0xaaaa, 0xaaaa, 0x5555, 0x5555,
  0xaaaa, 0xaaaa, 0x5555, 0x5555, 0xaaaa, 0xaaaa, 0x5555, 0x5555
};

/* In order to not save 16x16 for each pattern the data is compressed
 * in the following structure
 */
typedef struct {
  uint8_t row;    /* which row should by displayed */
  uint16_t pattern; /* pattern in this row */
}  
tableClockWordPatterns_t;

tableClockWordPatterns_t tableClockWordPatterns;

void updateDisplayISR();
#define TIMER_DELAY 64


void setup()
{
  Timer1.initialize(TIMER_DELAY); // Timer for updating pwm pins
  Timer1.attachInterrupt(updateDisplayISR);
  displayDriver.setPattern(myTestPattern);
}

void loop()
{

}

void updateDisplayISR()
{
  displayDriver.update();
}


