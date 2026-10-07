#pragma once

namespace Calendar
{
  // Ref is Jan 1 2026 00:00. uint32_t spans 136 years
  const int kRefYear    = 26;
  const int kRefMonth   = 1;
  const int kRefDate    = 1;
  const int kRefDayOfWeek = 4;  // Thu
  const int kDaysPerWeek = 7;

  const uint32_t kSecondsPerDay = 24UL*60UL*60UL;

  uint32_t DaysSinceReference(int Year, int Month, int Date);
  uint32_t SecondsSinceReference(int Year, int Month, int Date, int Hour24, int Minute);
  int DayOfWeek(int Year, int Month, int Date);
  int DaysInMonth(int month, int year);  // 1..12, '01..'99
  int DaysInYear(int year);  // '01..'99
  int TimeToMinutes(int h, int m);
  void MinutesToTime(int minutes, int& h, int& m);

  void Draw(bool full);  
}
