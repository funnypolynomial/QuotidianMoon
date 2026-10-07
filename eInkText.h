#pragma once

namespace eInkText
{
  // Insert text pixels into eInk row buffer
  int TextHeight(int scale);
  bool InsertText(uint8_t* pRowBuffer, int currentRow, int atRow, int atCol, const char* pText, eInkDisplay::Colour clr, int scale, bool progMem, bool underline = false, bool force5x6 = false);
}
