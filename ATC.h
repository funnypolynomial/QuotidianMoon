#pragma once

// Approximate Time Clock
namespace ATC
{
  // Time
  extern uint8_t  _DLS;
  // With DLS:
  uint8_t         GetHour24(uint8_t hour24); // returns hour24 (Std) as DLS
  uint8_t         SetHour24(uint8_t hour24); // returns hour24 (DLS) as Std

  extern bool     _TimeSet;
  extern uint8_t  _Hour24;    // 0..23 Standard time
  extern uint8_t  _Minute;    // 0..59
  extern uint8_t  _Year;      // 20xx
  extern uint8_t  _Month;     // 1..12
  extern uint8_t  _Date;      // 1..31
  extern uint8_t  _DayOfWeek; // 0..6 (Sun=0)

  void Init();
  bool Loop();
  void Set(uint8_t hour24, uint8_t minute);
  bool Adjust(int16_t minutesPastMidnight);

  extern void (*_MidnightCallback)();
}
