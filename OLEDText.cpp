#include <Arduino.h>
#include "Font5x6.h"
#include "OLED.h"
#include "OLEDText.h"

namespace OLEDText
{
  const int kCharWidth = 10;
  const int kCharGap   =  2;
  void SendChar(char ch, uint8_t& startColumn, bool upper)
  {
    // Send the char's column data to the current page
    // Upper or lower half of the char (5x6 but doubled)
    uint8_t base = upper?0:4;
    for (int pageCol = 0; pageCol < 10; pageCol++)
    {
      uint8_t data = 0;
      uint8_t mask = 0x10 >> (pageCol/2);
      for (int pageRow = 0; pageRow < 8; pageRow++)
      {
        uint8_t fontRowData = font5x6_GetRow(ch, pageRow/2 + base);
        uint8_t bit = (fontRowData & mask)?1:0;
        data |= (bit << pageRow);
      }
      OLED::PageColumn(data);
    }
    OLED::PageColumn(0x00); // gap between chars
    OLED::PageColumn(0x00);
    startColumn += kCharWidth + kCharGap; // add gap between chars
  }
  
  void SendLine(const char* pStr, uint8_t line, bool progmem, int width, int startCol)
  {
    // Send the string to the display, clip at width, start at startCol
    for (int half = 0; half < 2; half++)
    {
      const char* pCh = pStr;
      uint8_t col = 0;
      uint8_t ctr = kCharsPerLine;
      OLED::StartPage(2*line + half);
      while (col++ < startCol)
        OLED::PageColumn(0x00);
      while (pCh && ((progmem)?pgm_read_byte(pCh):*pCh) && ctr--)
        SendChar((progmem)?pgm_read_byte(pCh++):*pCh++, col, half == 0);
      while (col++ < width)
        OLED::PageColumn(0x00);
      OLED::EndPage();
    }
  }
  
  void DisplayLine(const char* pStr, uint8_t line, bool progmem, int width, int startCol)
  {
    // Display the string on two pages, line 0 or 1
    //   Line 0 Page 0
    //   Line 0 Page 1
    //   Line 1 Page 2
    //   Line 1 Page 3
    // Pads pStr with black to the end of the line
    // If pStr is longer, animates scrolling the line
    // Limits display blanked cols to width, if width is -ve, OLED::WIDTH is used
    // Starts at startCol. if startCol is WIDTH/2, auto-centres string
    int len = 0;
    if (width == -1)
      width = OLED::WIDTH;
    if (pStr)
      len = (int)(progmem)?::strlen_P(pStr):(::strlen(pStr));
    if (len <= kCharsPerLine)
    {
      if (startCol < 0)
        startCol = (OLED::WIDTH - len*(kCharWidth + kCharGap) - kCharGap)/2;
      SendLine(pStr, line, progmem, width, startCol);
    }
    else
    {
      for (int idx = 0; idx <= (len - kCharsPerLine); idx++)
      {
        SendLine(pStr + idx, line, progmem, width, 0);
        delay(500);
      }
    }
  }

}
