#include <Arduino.h>
#include "BBCFont.h"
#include "Config.h"
#include "eInkDisplay.h"
#include "eInkText.h"
#include "Font5x6.h"

namespace eInkText
{
int FontHeight(bool force5x6)
{
  if (force5x6)
    return 7;
#ifdef CFG_EINK_8x8_FONT
  return 8;
#else
  return 7;
#endif
}

int FontWidth(bool force5x6)
{
  if (force5x6)
    return 6;
#ifdef CFG_EINK_8x8_FONT
  return 8;
#else
  return 6;
#endif
}

int TextHeight(int scale)
{
  return scale*FontHeight(false);
}

uint8_t GetFontRow(char ch, int row, bool underline, bool force5x6)
{
  // Get the row defintion of the char, add underline
  uint8_t ul = (underline && row == FontHeight(force5x6) - 1)?0xFF:0x00;
  if (force5x6)
    return font5x6_GetRow(ch, row) | ul;
#ifdef CFG_EINK_8x8_FONT
  return bbc_GetFontRow(ch, row) | ul;
#else      
  return font5x6_GetRow(ch, row) | ul;
#endif
}

bool InsertText(uint8_t* pRowBuffer, int currentRow, int atRow, int atCol, const char* pText, eInkDisplay::Colour clr, int scale, bool progMem, bool underline, bool force5x6)
{
  // adds text pixels to the display at {atRow, atCol}
  // update the display buffer if the currentRow falls within the text rows, returns true if updated
  // Assumes correct Start* has been called, and matches the colour
  // If atRow/Col are -ve, they are indented from the bottom/left
  // It atCol is eInkDisplay::WIDTH/2 the text is centred horizontally
  int scaleY = scale;
  int len = (int)(progMem?strlen_P(pText):strlen(pText));  
  if (atRow < 0)
    atRow += eInkDisplay::HEIGHT - scaleY*FontHeight(force5x6); // bottom align
  if (atCol < 0)
    atCol += eInkDisplay::WIDTH - scale*len*FontWidth(force5x6); // left align
  else if (atCol == eInkDisplay::WIDTH/2)
    atCol -= scale*len*FontWidth(force5x6)/2; // hz centre
#ifdef CFG_EINK_UPRIGHT_DISPLAY
  currentRow = eInkDisplay::HEIGHT - currentRow - 1;
  atCol = eInkDisplay::WIDTH - atCol - 1;
  if (atRow <= currentRow && currentRow < (atRow + scaleY*FontHeight(force5x6)))
  {
    for (int i = len - 1; i >= 0; i--)
    {
      char ch = progMem?pgm_read_byte(pText + i):pText[i];
      uint8_t fontRow = GetFontRow(ch, (currentRow - atRow)/scaleY, underline, force5x6);
      uint8_t mask = 0x01;
      for (int j = FontWidth(force5x6) - 1; j >= 0 ; j--)
      {
        if (fontRow & mask)
          eInkDisplay::SetRowBufferAt(pRowBuffer, atCol - scale*(i*FontWidth(force5x6) + j), clr, scale);
        mask <<= 1;
      }
    }
    return true; 
  }
#else  
  if (atRow <= currentRow && currentRow < (atRow + scaleY*FontHeight(force5x6)))
  {
    for (int i = 0; i < len; i++)
    {
      char ch = progMem?pgm_read_byte(pText + i):pText[i];
      uint8_t fontRow = GetFontRow(ch, (currentRow - atRow)/scaleY, underline, force5x6);
      uint8_t mask = 0x01 << (FontWidth(force5x6) - 1);
      for (int j = 0; j < FontWidth(force5x6); j++)
      {
        if (fontRow & mask)
          eInkDisplay::SetRowBufferAt(pRowBuffer, atCol + scale*(i*FontWidth(force5x6) + j), clr, scale);
        mask >>= 1;
      }
    }
    return true; 
  }
#endif
  return false;
}
}
