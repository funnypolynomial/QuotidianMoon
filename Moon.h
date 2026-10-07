#pragma once

// Draw the moon on the eInk display

namespace Moon
{
  void Draw();
  uint32_t CalcReference(uint8_t days, uint8_t hour, uint8_t minute);
  int CalcPhase(int Year, int Month, int Date, int& Age);
  void CalcNextNewMoon(int Year, int Month, int Date, int& days, int& hour, int& minute);
  void GetPhaseName(int phase, const char*& Line0, const char*& Line1);
}

