#include <Arduino.h>
#include "ATC.h"
#include "Calendar.h"
#include "Config.h"
#include "Monitor.h"
#include "Pins.h"

namespace Monitor
{
// Consider an idealised scenario where it's dark from 9pm to 3am:
//       9pm   12am  3am
//       |     |     |
// ______#############_
// -day- ----night--- ^dawn
// At dawn, the duration of the dark period (6h) is halved and so the time is set to 3am, 
// i.e. 3 hours after (solar) midnight
//
// Now suppose we are so far West that sunrise & sunset are 1 hour later:
//        10pm 12am    4am
//         |   |       |
// ________#############_
// --day-- ----night--- ^dawn
// The dark duration is still 6h, half that is 3h, but we need to add the 1 hour in the zone
// to correctly set the time to 4am
//
// But note that we detect dawn at the end of an "interval" of daylight, so in fact the time is
// set as above, plus the interval.

const uint32_t kReadingPeriodMS = 60000UL; // every minute
const uint32_t kReadingsPerInterval = 10UL; // 10 minute interval

const uint32_t kMinutesPerInterval = kReadingPeriodMS*kReadingsPerInterval/60000UL;
const uint32_t kIntervalsPerHour = 60UL/kMinutesPerInterval;

uint8_t _ReadingStatesTotal = 0;  // total of 0's (dark) and 1's (light)
uint8_t _ReadingsCount      = 0;  // number of readings in interval
uint32_t _PrevMS            = 0;  // last time of reading
int16_t _DarkIntervals      = 0;  // number if dark intervals
// adjustment results, or 0/false
int16_t _NightTotalMinutes  = 0;  // lenght of night-time dark period
int     _DawnAtMinutes      = 0;  // time of dawn as minutes after midnight
int     _AdjustmentMinutes  = 0;  // how much ATC was shifted
bool    _Adjusted           = false; // true if adjustment made today
bool    _NewDay             = false; // redraw the Moon/Calendar
uint8_t _SkipCtr            = 0;     // if non-zero skip adjustment

// 24 hours of interval stats, 1 bit/interval, 1/0=light/dark
// ls bit of ls byte is most recent
const int kMaxIntervals = 24UL*kIntervalsPerHour;
uint8_t _PrevIntervalStats[kMaxIntervals/8UL];
int _TotalIntervals = 0;
// Current interval, ls bit is most recent
uint32_t _CurrentIntervalStats = 0;
void IntervalStats(int& ReadingsIntoInterval, int& IntervalsPerHour, int& TotalIntervals, int& TotalReadingsPerInterval)
{
  // return info about prev interval stats
  ReadingsIntoInterval = _ReadingsCount;
  IntervalsPerHour = (int)kIntervalsPerHour;
  TotalIntervals = _TotalIntervals;
  TotalReadingsPerInterval = kReadingsPerInterval;
}

bool PrevIntervalWasLight(int interval)
{
  // return the status of the prev interval. 0 is most recent
  if (interval < _TotalIntervals)
    return _PrevIntervalStats[interval/8] & (0x01 << (interval % 8));
  return false;
}

bool CurrentIntervalWasLight(int reading)
{
  // return the status of the reading in current interval. 0 is most recent
  if (reading < _ReadingsCount)
    return _CurrentIntervalStats & (1UL << reading);
  return false;
}

void AppendInterval(bool light)
{
  // push a new interval onto stats
  if (_TotalIntervals < kMaxIntervals)
    _TotalIntervals++;
  // shift bits up
  for (int b = (int)sizeof(_PrevIntervalStats) - 1; b >= 0; b--)
  {
    _PrevIntervalStats[b] <<= 1;
    if (b && (_PrevIntervalStats[b - 1] & 0x80))
      _PrevIntervalStats[b] |= 0x01;
  }
  // add new bit
  if (light)
    _PrevIntervalStats[0] |= 0x01;
}

void MidnightCallback()
{
  // Call-back from ATC at midnight, clear variables to indicate not adjustment yet
  _Adjusted = false;
  _DawnAtMinutes = 0;
  _AdjustmentMinutes = 0;
  _NightTotalMinutes = 0;
  _NewDay = true;
  if (_SkipCtr)
    _SkipCtr--;
}

void Init()
{
  // init vars
  ATC::_MidnightCallback = MidnightCallback;
  _ReadingStatesTotal = 0;
  _ReadingsCount = 0;
  _NewDay = _Adjusted = false;
  _NightTotalMinutes = 0;
  _AdjustmentMinutes = 0;
  _DawnAtMinutes = 0;
  _DarkIntervals = 0;
  pinMode(PIN_LDR, INPUT);
  _PrevMS = millis();
  _SkipCtr = 0;

  ::memset(_PrevIntervalStats, 0, sizeof(_PrevIntervalStats));
  _TotalIntervals = 0;
  _CurrentIntervalStats = 0;
}

void Loop()
{
  // Main loop, LDR readings, interval accumulation, ATC adjustment
  uint32_t nowMS = millis();
  if ((nowMS - _PrevMS) >= kReadingPeriodMS)
  {
    // reading every minute
    _PrevMS = nowMS;
    int reading = (analogRead(PIN_LDR) < CFG_MON_LDR_THRESHOLD)?0:1;
    _ReadingStatesTotal += reading;
    _CurrentIntervalStats <<= 1;
    _CurrentIntervalStats |= reading;
    _ReadingsCount++;
    if (_ReadingsCount >= kReadingsPerInterval)
    {
      // a full interval of readings
      uint8_t ReadingStateAverage = (_ReadingStatesTotal <= (_ReadingsCount/2))?0:1; // more than half dark?
      AppendInterval(ReadingStateAverage);

      _ReadingStatesTotal = 0;
      _ReadingsCount = 0;
      if (ReadingStateAverage == 0)
        _DarkIntervals++; // still dark, still night
      else
      {
        // first light interval
        uint16_t TotalDarkMinutes = _DarkIntervals*kMinutesPerInterval;
        if (CFG_MON_MIN_DARK_MINUTES <= TotalDarkMinutes && TotalDarkMinutes <= CFG_MON_MAX_DARK_MINUTES)
        {
          // Dawn! Assume midnight was in the middle of the dark intervals
          int16_t MinutesPastMidnight = TotalDarkMinutes/2UL;
#ifdef CFG_MON_MINUTES_INTO_ZONE
          // +ve value is West, later
          MinutesPastMidnight += CFG_MON_MINUTES_INTO_ZONE;
#endif            
          MinutesPastMidnight += kMinutesPerInterval; // we are at the end of the first daylight interval
          int AdjustmentDeltaMinutes = MinutesPastMidnight - Calendar::TimeToMinutes(ATC::_Hour24, ATC::_Minute);
          if (abs(AdjustmentDeltaMinutes) <= CFG_MON_MAX_ADJUST_MINUTES && _SkipCtr == 0 && Config::_MonitorOn)
          {
            // Adjust, record the data
            _DawnAtMinutes = Calendar::TimeToMinutes(ATC::GetHour24(ATC::_Hour24), ATC::_Minute) - kMinutesPerInterval;
            _NightTotalMinutes = TotalDarkMinutes;
            _AdjustmentMinutes = AdjustmentDeltaMinutes;
            _Adjusted = ATC::Adjust(MinutesPastMidnight);
          }
        }
        _DarkIntervals = 0; // restart
      }
    }
  }
}

bool ToggleSkipNextAdjustment()
{
  // Sets the ctr to 2 (or 0's if non-zero) Returns true is skip is on
  // At midnight it is decremented if non-zero
  // Adjustment is skipped if non-zero
  // Thus if you know home lighting has likely shifted the detection of
  // sunset and hence the time adjustment, calling this will mean later
  // that night at midnight it will go to 1, and the adjustment will be
  // skipped. The following midnight it will go back to 0 and the 
  // adjustment will proceed as normal.
  _SkipCtr = _SkipCtr?0:2;
  return _SkipCtr != 0;
}

bool SkippingNextAdjustment()
{
  // True is skipping next adjustment
  return _SkipCtr == 2;
}


int RawLDR(bool& isDay)
{
  // Return raw LDR and set if it's dark or light
  int ldr = analogRead(PIN_LDR);
  isDay = ldr > CFG_MON_LDR_THRESHOLD;
  return ldr;
}
}
