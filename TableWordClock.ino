/*
Toughts:
 Einstellen mittels binÃ¤rcode dabei blinken dian oder fen auf
 */
/*
 * !!!!!!!!
 * This sketch needs the following non-standard libraries (install them in the Arduino library directory):
 * TimerOne: http://www.arduino.cc/playground/Code/Timer1
 * !!!!!!!!
 
 */
#include <TimerOne.h>
#include <Time.h>
#include "TableWordClock.h"
#include "DisplayDriver.h"

/* ************************ Defines ************************************ */
#define DISPLAY_REFRESHTIME      50*1000L   /* Timer1 perdiod is measured in microseconds (10e-6). Don't omit the "L", if so it will not work */
#define TIME_UPDATE_DELAY_TIME   10*1000   /* time should be updated every 10 seconds. Delay is given in milli seconds (10e-3) */

#define DEBUG
/* If DEBUG_RUNTIME_MEASUREMENT is defined PIN RUNTIME_ISR_PIN will go high when application enters timer ISR and go
 * low when ISR is left. PIN RUNTIME_LOOP_PIN will go high when loop function is entered and low right before the final
 * delay() call.
 */
#define DEBUG_RUNTIME_MEASUREMENT
#ifdef DEBUG_RUNTIME_MEASUREMENT
#define RUNTIME_ISR_PIN  2
#define RUNTIME_LOOP_PIN 3
#endif

displayPattern_t clockPattern = {
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

DisplayDriver displayDriver;

/* patterns for words. Should be maybe reordered to make show hours easier */
tableClockWordPattern_t tableClockWordPattern[] = 
{
  { 0, 0xfc00}, /* (xxx minutes/quarter) to ->     1111 1100 0000 0000 */
  { 0, 0x03c0}, /* 2(0 minutes to) ->              0000 0011 1100 0000 */
  { 0, 0x003f}, /* 10 (minutes) (to) ->            0000 0000 0011 1111 */
  { 2, 0xff00}, /* one quearter (to) ->            1111 1111 0000 0000 */
  { 2, 0x003f}, /* (xxx) minutes (to) ->           0000 0000 0011 1111 */
  { 4, 0xfc00}, /* 1x (o'clock) ->                 1111 1100 0000 0000 */
  { 4, 0x03c0}, /* x1 and 1 (o'clock) ->           0000 0011 1100 0000 */
  { 4, 0x0030}, /* 2 (o'clock) ->                  0000 0000 0011 0000 */
  { 4, 0x000f}, /* (1)2 (o'clock) ->               0000 0000 0000 1111 */
  { 6, 0xfc00}, /* 3 (o'clock) ->                  1111 1100 0000 0000 */
  { 6, 0x03c0}, /* 4 (o'clock) ->                  0000 0011 1100 0000 */
  { 6, 0x003f}, /* 6 (o'clock) ->                  0000 0000 0011 1111 */
  { 8, 0xf000}, /* 5 (o'clock) ->                  1111 0000 0000 0000 */
  /* 0 o'lock is missing ??? */
  { 8, 0x003f}, /* 9 (o'clock) ->                  0000 0000 0011 1111 */
  {10, 0xf000}, /* 7 (o'clock) ->                  1111 0000 0000 0000 */
  {10, 0x0f00}, /* 8 (o'clock) ->                  0000 1111 0000 0000 */
  {10, 0x00ff}, /* (xx) o'clock ->                 0000 0000 1111 1111 */
  {12, 0xf000}, /* 1(quarter after) ->             1111 0000 0000 0000 */
  {12, 0x0f00}, /* 2(0) mins after) ->             0000 1111 0000 0000 */
  {12, 0x003f}, /* (x)0 (mins after) ->            0000 0000 0011 1111 */
  {14, 0xf000}, /* (1) quater (after) ->           1111 0000 0000 0000 */
  {14, 0x0fc0}, /* x:30 (after) ->                 0000 1111 1100 0000 */
  {14, 0x003f}, /* (x) mins (after) ->             0000 0000 0011 1111 */
};

hoursToWordPatternMapping_t hoursToWordPatternMapping[] = {
  { 0,  5},  /* ten, 0 o'clock is for now also display as 12 o'clock */
  { 0,  8},  /* + two */
  { 1,  6},  /* one */
  { 2,  7},  /* two */
  { 3,  9},  /* three */
  { 4, 10},  /* four */
  { 5, 12},  /* five */
  { 6, 11},  /* six */
  { 7, 14},  /* seven */
  { 8, 15},  /* eight */
  { 9, 13},  /* nine */
  {10,  5},  /* ten */
  {11,  5},  /* ten */
  {11,  6},  /* + one */
  {12,  5},  /* ten */
  {12,  8}   /* + two */
};

minutesToWordPatternMapping_t minutesToWordPatternMapping[] = {
  {10, 14, 19}, /* ten */
  {10, 14, 22}, /* minutes */
  {15, 19, 17}, /* one */
  {15, 19, 20}, /* quarter */
  {20, 29, 18}, /* two */
  {20, 29, 19}, /* ten */
  {20, 29, 22}, /* minutes */
  {30, 39, 21}, /* half */
  {40, 44,  0}, /* before */
  {40, 44,  1}, /* two */
  {40, 44,  2}, /* ten */
  {40, 44,  4}, /* minutes */
  {45, 49,  0}, /* before */
  {45, 49,  3}, /* one quarter */
  {50, 59,  0}, /* before */
  {50, 59,  2}, /* ten */
  {50, 59,  4}, /* minutes */
};

time_t currentTime;

/* ************************ Function prototypes ************************ */
void updateDisplayISR();
#ifdef DEBUG
void digitalClockDisplay();
void serialPrintBinary(uint16_t);
#endif

void setup()
{  
  long test;
  
#ifdef DEBUG
  Serial.begin(9600);
#endif

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Timer1.initialize(DISPLAY_REFRESHTIME); /* initialize timer1, and set period for cyclic update of display content  */
  Timer1.attachInterrupt(updateDisplayISR);
  currentTime = now();

#ifdef DEBUG_RUNTIME_MEASUREMENT
  pinMode(RUNTIME_ISR_PIN, OUTPUT);
  digitalWrite(RUNTIME_ISR_PIN, LOW);
  pinMode(RUNTIME_LOOP_PIN, OUTPUT);
  digitalWrite(RUNTIME_LOOP_PIN, LOW);
#endif

  setTime(16, 9, 10 ,19 , 8, 2014);
}

/** TODO:
 * - Check type for delay function
 */

void loop()
{
  int currentHour, currentMinute;
 
#if 1
  static int ledstate = LOW;
  if (ledstate == LOW) ledstate = HIGH;
  else ledstate = LOW;
  digitalWrite(LED_BUILTIN, ledstate);
#endif

#ifdef DEBUG_RUNTIME_MEASUREMENT
  digitalWrite(RUNTIME_LOOP_PIN, HIGH);
#endif

  currentTime = now();
  currentHour = hour(currentTime);
  currentMinute = minute(currentTime);
  uint8_t row;

  /* erase clock pattern */
  memset(clockPattern, 0x0000, sizeof(displayPattern_t));
  
  /* set minutes */
  for (int i=0; i < sizeof(minutesToWordPatternMapping)/sizeof(minutesToWordPatternMapping_t); i++)
  {
    if ( ( currentMinute >= minutesToWordPatternMapping[i].from ) && ( currentMinute <= minutesToWordPatternMapping[i].to ) )
    {
      row = minutesToWordPatternMapping[i].row;
      clockPattern[tableClockWordPattern[row].displayRow] |= tableClockWordPattern[row].pattern;
      clockPattern[tableClockWordPattern[row].displayRow + 1] |= tableClockWordPattern[row].pattern;
    }
  }
  
   /* If minutes are greater than 39, clock switches to XXX before YYY. Thus we need to increase hour 
   * by one */
  if (currentMinute >= 40)
    currentHour++;
  /* make sure clock stays within 0 ... 11 */
  currentHour = currentHour % 12;
  
  for (int i=0; i < sizeof(hoursToWordPatternMapping)/sizeof(hoursToWordPatternMapping_t); i++)
  {
    if ( currentHour ==  hoursToWordPatternMapping[i].hour)
    {
      row = hoursToWordPatternMapping[i].row;
      clockPattern[tableClockWordPattern[row].displayRow] |= tableClockWordPattern[row].pattern;
      clockPattern[tableClockWordPattern[row].displayRow + 1] |= tableClockWordPattern[row].pattern;
    }
  }
  /* display o'clock always */
  row = tableClockWordPattern[16].displayRow;
  clockPattern[row] |= tableClockWordPattern[16].pattern;
  clockPattern[row + 1] |= tableClockWordPattern[16].pattern;
  
  displayDriver.setPattern(clockPattern);
  
#ifdef DEBUG
  digitalClockDisplay();
#endif
#ifdef DEBUG_RUNTIME_MEASUREMENT
  digitalWrite(RUNTIME_LOOP_PIN, LOW);
#endif
  delay(TIME_UPDATE_DELAY_TIME);  
}

void updateDisplayISR()
{
#ifdef DEBUG_RUNTIME_MEASUREMENT
  digitalWrite(RUNTIME_ISR_PIN, HIGH);
#endif
  displayDriver.update();
#ifdef DEBUG_RUNTIME_MEASUREMENT
  digitalWrite(RUNTIME_ISR_PIN, LOW);
#endif
}

#ifdef DEBUG
void digitalClockDisplay(){
  for (int i=0; i<16; i++)
  {
    serialPrintBinary(clockPattern[i]);
    Serial.print(" -> ");
    Serial.print(clockPattern[i], BIN);
    Serial.println();
  }
  // digital clock display of the time
  Serial.print(hour());
  Serial.print(minute());
  Serial.print(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.println();
  Serial.println();
}
#endif

#ifdef DEBUG
void serialPrintBinary(uint16_t content)
{
  for (int i=0; i<16; i++)
  {
    if ((i%2) == 0) Serial.print(" ");
    content & 0x8000 ? Serial.print('1') : Serial.print('0');
    content = content << 1;
  }
}
#endif
