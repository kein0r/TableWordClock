#include <Wire.h>
#include "DS3231RTC.h"

#define DS3231_I2C_ID 0x68

DS3231::DS3231()
{
  Wire.begin();
}

// PUBLIC FUNCTIONS
time_t DS3231::get() // Aquire data from buffer and convert to time_t
{
  tmElements_t tm;
  read(tm);
  return(makeTime(tm));
}

float DS3231::getTemp()
{
  float temp3231;
  Wire.beginTransmission(DS3231_I2C_ID);
  Wire.write(0x11);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ID, 2);
  if(Wire.available())
  {
    int tMSB = Wire.read(); //2's complement int portion
    int tLSB = Wire.read(); //fraction portion
    temp3231 = (tMSB & 127); //do 2's math on Tmsb
    temp3231 += ( (tLSB >> 6) * 0.25 ); //only care about bits 7 & 8
  }
  else
  {
    //oh noes, no data!
  }
  return temp3231;
}

void DS3231::set(time_t t)
{
  tmElements_t tm;
  breakTime(t, tm);
  tm.Second |= 0x80; // stop the clock
  write(tm);
  tm.Second &= 0x7f; // start the clock
  write(tm);
}

// Aquire data from the RTC chip in BCD format
void DS3231::read( tmElements_t &tm)
{
  Wire.beginTransmission(DS3231_I2C_ID);
  Wire.write(0x00);
  Wire.endTransmission();

  // request the 7 data fields (secs, min, hr, dow, date, mth, yr)
  Wire.requestFrom(DS3231_I2C_ID, tmNbrFields);

  tm.Second = bcd2dec(Wire.read() & 0x7f);
  tm.Minute = bcd2dec(Wire.read() );
  tm.Hour = bcd2dec(Wire.read() & 0x3f); // mask assumes 24hr clock
  tm.Wday = bcd2dec(Wire.read() );
  tm.Day = bcd2dec(Wire.read() );
  tm.Month = bcd2dec(Wire.read() );
  tm.Year = y2kYearToTm((bcd2dec(Wire.read())));
}

// Aquire minimum data to show time
void DS3231::readFast( tmElements_t &tm)
{
  Wire.beginTransmission(DS3231_I2C_ID);
  Wire.write(0x00);
  Wire.endTransmission();

  // request only the first 3 data fields (secs, min, hr)
  Wire.requestFrom(DS3231_I2C_ID, 3);

  tm.Second = bcd2dec(Wire.read() & 0x7f);
  tm.Minute = bcd2dec(Wire.read() );
  tm.Hour = bcd2dec(Wire.read() & 0x3f); // mask assumes 24hr clock
}

void DS3231::write(tmElements_t &tm)
{
  Wire.beginTransmission(DS3231_I2C_ID);
  Wire.write(0x00); // reset register pointer

  Wire.write(dec2bcd(tm.Second)) ;
  Wire.write(dec2bcd(tm.Minute));
  Wire.write(dec2bcd(tm.Hour)); // sets 24 hour format
  Wire.write(dec2bcd(tm.Wday));
  Wire.write(dec2bcd(tm.Day));
  Wire.write(dec2bcd(tm.Month));
  Wire.write(dec2bcd(tmYearToY2k(tm.Year)));

  Wire.endTransmission();
}
// PRIVATE FUNCTIONS

// Convert Decimal to Binary Coded Decimal (BCD)
uint8_t DS3231::dec2bcd(uint8_t num)
{
  return ((num/10 * 16) + (num % 10));
}

// Convert Binary Coded Decimal (BCD) to Decimal
uint8_t DS3231::bcd2dec(uint8_t num)
{
  return ((num/16 * 10) + (num % 16));
}

DS3231 RTC = DS3231(); // create an instance for the user

