#pragma once

// monitor the LDR, detect dawn, adjust ATC
namespace Monitor
{
  void Init();
  void Loop();
  bool ToggleSkipNextAdjustment();
  bool SkippingNextAdjustment();
  int RawLDR(bool& isDay);

  // stats
  void IntervalStats(int& ReadingsIntoInterval, int& IntervalsPerHour, int& TotalIntervals, int& TotalReadingsPerInterval);
  bool PrevIntervalWasLight(int interval);
  bool CurrentIntervalWasLight(int reading);

  extern int16_t _DarkIntervals;
  extern int16_t _NightTotalMinutes;
  extern bool    _Adjusted;
  extern int     _DawnAtMinutes;
  extern int     _AdjustmentMinutes;
  extern bool    _NewDay;
}
