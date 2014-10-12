/*
Toughts:
 Einstellen mittels binÃ¤rcode dabei blinken dian oder fen auf
 */
/*
 * !!!!!!!!
 * This sketch needs the following non-standard libraries (install them in the Arduino library directory):
 * TimerOne: http://www.arduino.cc/playground/Code/Timer1
 * Time: http://playground.arduino.cc/Code/Time (used in DS2321RTC lib)
 * !!!!!!!!
 
 * Data (marked D on DS3231 boards) must be connected to SDA (or Analog 4)
 * Clock (marked C on DS3231 boards) must be connected to SCL (or Analog 5)
 */
#include <TimerOne.h>
#include <Time.h>
#include <Wire.h>
#include "TableWordClock.h"
#include "DisplayDriver.h"
#include "DS3231RTC.h"

/* 
 * Latest measurement:
 * ISR runs approx 2.3ms
 * loop runs 320uS
 * DISPLAY_REFRESHTIME was chosen such that MAIN_LOOP runs approx with specified time.
 * DisplayRefreshTime 5ms -> loop 680ms
 * DisplayRefreshTime 4ms -> loop 740ms
 * DisplayRefreshTime 3ms -> loop 900ms
 */

/* ************************ Defines ************************************ */
#define DISPLAY_REFRESHTIME      4*1000L   /* Timer1 perdiod is measured in microseconds (10e-6). Don't omit the "L", if so it will not work */
#define TIME_UPDATE_DELAY_TIME   500        /* Delay time between two time updated (i.e. read from RTC). Delay is given in milli seconds (10e-3) */

#define CLOCKSET_HOUR_INCREMENT_PIN   2  /* pin 2 uses int.0 */
#define CLOCKSET_HOUR_PIN_INT         0
#define CLOCKSET_MINUTE_INCREMENT_PIN 3  /* pin 3 uses int.1 */
#define CLOCKSET_MINUTE_PIN_INT       1

/*  code to process time sync messages from the serial port   */
#define TIME_HEADER  "T"   // Header tag for serial time sync message
#define TIME_TIMEZONE  8L   // UTC offset to be added when setting time via PC

#define SERIAL_BAUDRATE  115200

//#define DEBUG

/* If DEBUG_RUNTIME_MEASUREMENT is defined PIN RUNTIME_ISR_PIN will go high when application enters timer ISR and go
 * low when ISR is left. PIN RUNTIME_LOOP_PIN will go high when loop function is entered and low right before the final
 * delay() call.
 */
//#define DEBUG_RUNTIME_MEASUREMENT
#ifdef DEBUG_RUNTIME_MEASUREMENT
#define RUNTIME_ISR_PIN  0
#define RUNTIME_LOOP_PIN 4
#endif

displayPattern_t clockPattern = {
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

DisplayDriver displayDriver;

/* patterns for words. Should be maybe reordered to make show hours easier */
static const tableClockWordPattern_t tableClockWordPattern[] = 
{
  {0, 0xfc00}, /* (xxx minutes/quarter) to ->     1111 1100 0000 0000 */
  {0, 0x03c0}, /* 2(0 minutes to) ->              0000 0011 1100 0000 */
  {0, 0x003f}, /* 10 (minutes) (to) ->            0000 0000 0011 1111 */
  {1, 0xff00}, /* one quearter (to) ->            1111 1111 0000 0000 */
  {1, 0x003f}, /* (xxx) minutes (to) ->           0000 0000 0011 1111 */
  {2, 0xfc00}, /* 1x (o'clock) ->                 1111 1100 0000 0000 */
  {2, 0x03c0}, /* x1 and 1 (o'clock) ->           0000 0011 1100 0000 */
  {2, 0x0030}, /* 2 (o'clock) ->                  0000 0000 0011 0000 */
  {2, 0x000f}, /* (1)2 (o'clock) ->               0000 0000 0000 1111 */
  {3, 0xfc00}, /* 3 (o'clock) ->                  1111 1100 0000 0000 */
  {3, 0x03c0}, /* 4 (o'clock) ->                  0000 0011 1100 0000 */
  {3, 0x003f}, /* 6 (o'clock) ->                  0000 0000 0011 1111 */
  {4, 0xf000}, /* 5 (o'clock) ->                  1111 0000 0000 0000 */
  /* 0 o'lock is missing ??? */
  {4, 0x00fc}, /* 9 (o'clock) ->                  0000 0000 1111 1100 */
  {5, 0xf000}, /* 7 (o'clock) ->                  1111 0000 0000 0000 */
  {5, 0x0f00}, /* 8 (o'clock) ->                  0000 1111 0000 0000 */
  {5, 0x00ff}, /* (xx) o'clock ->                 0000 0000 1111 1111 */
  {6, 0xf000}, /* 1(quarter after) ->             1111 0000 0000 0000 */
  {6, 0x0f00}, /* 2(0) mins after) ->             0000 1111 0000 0000 */
  {6, 0x003f}, /* (x)0 (mins after) ->            0000 0000 0011 1111 */
  {7, 0xf000}, /* (1) quater (after) ->           1111 0000 0000 0000 */
  {7, 0x0fc0}, /* x:30 (after) ->                 0000 1111 1100 0000 */
  {7, 0x003f}, /* (x) mins (after) ->             0000 0000 0011 1111 */
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

boolean updateTimeOfRTC = false;    /* whenever this variable is set to true current time will be writte to RTC in main loop */
tmElements_t currentTime;

/* ************************ Function prototypes ************************ */

void incrementHourISR();
void incrementMinuteISR();
void updateDisplayISR();
#ifdef DEBUG
void digitalClockDisplay(tmElements_t &t);
void serialPrintBinary(uint16_t);
#endif

void setup()
{
  Serial.begin(SERIAL_BAUDRATE);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  /* configure input buttons */
  pinMode(CLOCKSET_HOUR_INCREMENT_PIN, INPUT_PULLUP);
  pinMode(CLOCKSET_MINUTE_INCREMENT_PIN, INPUT_PULLUP);
  /* clear possible interrupt flag due to above pull-up set prior to enabling the ISRs */
  EIFR = 0x03;
  attachInterrupt(CLOCKSET_HOUR_PIN_INT, incrementHourISR, RISING);
  attachInterrupt(CLOCKSET_MINUTE_PIN_INT, incrementMinuteISR, RISING);
  
  Timer1.initialize(DISPLAY_REFRESHTIME); /* initialize timer1, and set period for cyclic update of display content  */
  Timer1.attachInterrupt(updateDisplayISR);

#ifdef DEBUG_RUNTIME_MEASUREMENT
  pinMode(RUNTIME_ISR_PIN, OUTPUT);
  digitalWrite(RUNTIME_ISR_PIN, LOW);
  pinMode(RUNTIME_LOOP_PIN, OUTPUT);
  digitalWrite(RUNTIME_LOOP_PIN, LOW);
#endif

  /* set time manually. Use date +%s to get it in UTC format. You need to add local time zone. */
  /* RTC.set(1408852621L + (8*60*60L)); */
}

unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013 

  if(Serial.find(TIME_HEADER)) {
     pctime = Serial.parseInt();
     /* add local time zone */
     pctime += (TIME_TIMEZONE*60*60L);
     return pctime;
     if( pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
       pctime = 0L; // return 0 to indicate that the time is not valid
     }
  }
  return pctime;
}

void loop()
{  
  uint8_t row;
   
  static int ledstate = LOW;
  if (ledstate == LOW) ledstate = HIGH;
  else ledstate = LOW;
  digitalWrite(LED_BUILTIN, ledstate);

#ifdef DEBUG_RUNTIME_MEASUREMENT
  digitalWrite(RUNTIME_LOOP_PIN, HIGH);
#endif

  /* to be able to set time from PC by using "date +T%s > /dev/ttyUSB0" */
  if (Serial.available()) {
    Timer1.detachInterrupt();
    time_t t = processSyncMessage();
    if (t != 0) {
      breakTime(t, currentTime);
      updateTimeOfRTC = true;
    }
    Timer1.attachInterrupt(updateDisplayISR);
  }

  if (updateTimeOfRTC)
  {
#ifdef DEBUG
    Serial.println("Time was updated!");
#endif
    RTC.write(currentTime);
    updateTimeOfRTC = false;
  }
  else
  {
    RTC.read(currentTime);
  }

  /* erase clock pattern */
  memset(clockPattern, 0x0000, sizeof(displayPattern_t));
  
  /* set minutes */
  for (int i=0; i < sizeof(minutesToWordPatternMapping)/sizeof(minutesToWordPatternMapping_t); i++)
  {
    if ( ( currentTime.Minute >= minutesToWordPatternMapping[i].from ) && ( currentTime.Minute <= minutesToWordPatternMapping[i].to ) )
    {
      row = minutesToWordPatternMapping[i].row;
      clockPattern[tableClockWordPattern[row].displayRow] |= tableClockWordPattern[row].pattern;
    }
  }
  
   /* If minutes are greater than 39, clock switches to XXX before YYY. Thus we need to increase hour 
   * by one */
  if (currentTime.Minute >= 40)
    currentTime.Hour++;
  /* make sure clock stays within 0 ... 11 */
  currentTime.Hour %= 12;
  
  for (int i=0; i < sizeof(hoursToWordPatternMapping)/sizeof(hoursToWordPatternMapping_t); i++)
  {
    if ( currentTime.Hour ==  hoursToWordPatternMapping[i].hour)
    {
      row = hoursToWordPatternMapping[i].row;
      clockPattern[tableClockWordPattern[row].displayRow] |= tableClockWordPattern[row].pattern;
    }
  }
  if (currentTime.Minute >= 40)
    currentTime.Hour--;
  /* display o'clock always */
  row = tableClockWordPattern[16].displayRow;
  clockPattern[row] |= tableClockWordPattern[16].pattern;
  
  displayDriver.setPattern(clockPattern);  
  
#ifdef DEBUG
  digitalClockDisplay(currentTime);
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

void incrementHourISR()
{
  if (updateTimeOfRTC == false)
  {
    currentTime.Hour++;
    currentTime.Hour %= 12;
    updateTimeOfRTC = true;
  }
}

void incrementMinuteISR(){
  if (updateTimeOfRTC == false)
  {
    currentTime.Minute++;
    currentTime.Minute %= 60;
    updateTimeOfRTC = true;
  }
}


#ifdef DEBUG
void digitalClockDisplay(tmElements_t &t){
  for (int i=0; i<16; i++)
  {
    serialPrintBinary(clockPattern[i]);
    Serial.println();
  }
  // digital clock display of the time
  Serial.print(t.Hour);
  Serial.print(":");
  Serial.print(t.Minute);
  Serial.print(":");
  Serial.print(t.Second);
/*  Serial.print(" ");
  Serial.print(t.Day);
  Serial.print(" ");
  Serial.print(t.Month);
  Serial.print(" ");
  Serial.print(t.Year); */
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
