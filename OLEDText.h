#pragma once

// Draw text on OLED display as doubled 5x6 chars

namespace OLEDText
{
  // supports two lines of 10 chars
  const int kCharsPerLine = 10;
  void DisplayLine(const char* pStr, uint8_t line, bool progmem, int width = -1, int startCol = 0);
}
