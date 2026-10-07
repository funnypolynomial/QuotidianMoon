#include <Arduino.h>
#include "ATC.h"
#include "Calendar.h"
#include "Config.h"

namespace ATC
{
  // Minute adjusted for baseline drift
  // Note: might be useful as a run-time configurable item, or even values for each season
  const uint32_t kAdjustedMinuteMS = 60000UL
  #ifdef CFG_ATC_LOSS_PER_MINUTE_MS
   - CFG_ATC_LOSS_PER_MINUTE_MS
  #endif
  ;
  bool     _TimeSet = false;
  uint8_t  _DLS = 0;
  uint8_t  _Hour24 = 12; // standard, no DLS
  uint8_t  _Minute = 0;
  uint8_t  _Year = 26; // 20xx
  uint8_t  _Month = 1;     // 1..12
  uint8_t  _Date = 1;      // 1..31
  uint8_t  _DayOfWeek = 0; // 0..6 (Sun=0)

  uint32_t _lastMS = 0;
  void (*_MidnightCallback)() = NULL;

  uint8_t GetHour24(uint8_t hour24)
  {
    // return Std hour24 as DLS
    return ((_DLS)?hour24 + 1:hour24) % 24;
  }
  
  uint8_t SetHour24(uint8_t hour24)
  {
    // return DLS hour24 as Std
    hour24 = (_DLS)?hour24 - 1:hour24;
    if (hour24 > 23)
      hour24 = 23;
    return hour24;
  }

  void AdvanceDate()
  {
    // Advance the date 1 day, and save it
    _DayOfWeek++;
    if (_DayOfWeek > 6)
      _DayOfWeek = 0;
    _Date++;
    if (_Date > Calendar::DaysInMonth(_Month, _Year))
    {
      _Date = 1;
      _Month++;
      if (_Month > 12)
      {
        _Month = 1;
        _Year++;
        if (_Year > 99)
          _Year = 0;
      }
    }
    // record the most recent date
    // 100,000 EEPROM write/erase cycles, one a day = 270 years
    Config::Save();
  }
  
  void Init()
  {
    // initialise vars
    _DLS = 0;
    _Hour24 = 12;
    _Minute = 0;
#ifdef DEBUG  
    _TimeSet = true;    
#else
    _TimeSet = false;    
#endif      
    _Year = Calendar::kRefYear;
    _Month = Calendar::kRefMonth;
    _Date = Calendar::kRefDate;
    _DayOfWeek = Calendar::kRefDayOfWeek;
    
    _lastMS = millis();
  }
  
  bool Loop()
  {
    // check elapsed milliseconds and update time, true if updated
    bool updated = false;
    uint32_t thisMS = millis();
    uint32_t diffMS = thisMS - _lastMS;
    if (diffMS >= kAdjustedMinuteMS)
    {
      updated = true;
      // at least a minute has passed
      while (diffMS >= kAdjustedMinuteMS)
      {
        _Minute++;
        if (_Minute > 59)
        {
          _Minute = 0;
          _Hour24++;

#ifdef CFG_ATC_BLINK_LED_ON_HOUR
          digitalWrite(LED_BUILTIN, HIGH);
          delay(5000);
          digitalWrite(LED_BUILTIN, LOW);
#endif          
          if (_Hour24 > 23)
          {
            _Hour24 = 0;
            AdvanceDate();
            if (_MidnightCallback)
              _MidnightCallback();
          }      
        }
        diffMS -= kAdjustedMinuteMS;
      }
      _lastMS = thisMS - diffMS; // restart on the minute
    }
    return updated;
  }

  void Set(uint8_t hour24, uint8_t minute)
  {
    _Hour24 = SetHour24(hour24);
    _Minute = minute;
    _TimeSet = true;
    _lastMS = millis();
  }

  bool Adjust(int16_t minutesPastMidnight)
  {
    // adjusts the time
    if (0 < minutesPastMidnight && minutesPastMidnight < 24*60)
    {
      _Hour24 = minutesPastMidnight / 60;
      _Minute = minutesPastMidnight % 60;
      return true;
    }
    return false;
  }
};
