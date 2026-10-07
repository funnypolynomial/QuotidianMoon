#include <Arduino.h>
#include "ATC.h"
#include "Calendar.h"
#include "Config.h"
#include "eInkDisplay.h"
#include "eInkText.h"
#include "OLED.h"
#include "OLEDText.h"

namespace Calendar
{
uint32_t DaysSinceReference(int Year, int Month, int Date)
{
  // Return the number of days since the reference date
  uint32_t days = 0;
  // whole days
  while (Date-- > kRefDate)
    days++;
  // whole months
  while (Month-- > kRefMonth)
    days += DaysInMonth(Month, Year);
  // whole years
  while (Year-- > kRefYear)
    days += DaysInYear(Year);
  return days;
}

uint32_t SecondsSinceReference(int Year, int Month, int Date, int Hour24, int Minute)
{
  // Return the number of seconds since the reference date
  uint32_t seconds = 0;
  // seconds into day
  seconds += 60UL*TimeToMinutes(Hour24, Minute);
  // days
  seconds += kSecondsPerDay * DaysSinceReference(Year, Month, Date);
  return seconds;
}

int DayOfWeek(int Year, int Month, int Date)
{
  // Day of week of date, 0=Sunday
  return (int)((DaysSinceReference(Year, Month, Date) + kRefDayOfWeek) % kDaysPerWeek);
}


int DaysInMonth(int month, int year)  // 1..12, '01..'99
{
  // Return days in the month, of the year
  // <Days-in-Month>-28 encoded as 2bits/Month:
  //                             0b00DDNN...          ...FFJJ00
  //                                 eeoo                eeaa
  //                                 ccvv                bbnn    
  const uint32_t MONTH_LENGTHS = 0b0011101110111110111011001100UL;
  int Increment = (MONTH_LENGTHS >> (month*2) & 0x03);
  if (year % 4 || Increment)
    return 28 + Increment;
  else
    return 29;// Leap year AND Feb
}

int DaysInYear(int year)  // '01..'99
{
  // Days in the year
  return 337 + DaysInMonth(2, year);
}
  
int TimeToMinutes(int h, int m)
{
  // Convert time to minutes after midnight
  return h*60 + m;
}

void MinutesToTime(int minutes, int& h, int& m)
{
  // Convert minutes after midnight to time 
  while (minutes < 0)
    minutes += 24*60;
  while (minutes > 24*60)
    minutes -= 24*60;
  h = minutes / 60;
  m = minutes % 60;
}

#ifdef CFG_CAL_SOLID_LINES
const bool kSolidLines = true;
#else
const bool kSolidLines = false;
#endif
#ifdef CFG_EINK_UPRIGHT_DISPLAY
const int kBump = 1;
#else
const int kBump = 0;
#endif
const int kScale = 2;
const int kFontWidth = 6;
const int kCellCols = kDaysPerWeek;
const int kCellRows = 8;
const int kCellWidth = 28;
const int kCellHeight = 24;
const int kRowWidth = kCellCols*kCellWidth + 1;
const int kColHeight = kCellRows*kCellHeight + 1;
// top-left corner of the calendar
const int kOffsetLeft = (eInkDisplay::WIDTH - kRowWidth)/2;
const int kOffsetTop = (eInkDisplay::HEIGHT - kColHeight)/2;
// offset of text into cells
const int kCellTextOffsetRow  = (kCellHeight - kScale*kFontWidth)/2;           // from top
const int kCellTextOffsetCol1 = (kCellWidth - 1*kScale*kFontWidth)/2 + kBump;   // from left, 1 char
const int kCellTextOffsetCol2 = (kCellWidth - 2*kScale*kFontWidth)/2 + kBump;   // from left, 2 chars
const int kCellTextOffsetCol3 = (kRowWidth - 15*kScale*kFontWidth)/2 + kBump;   // from left, date, 15 chars
const int kCellHighlightThickness = 3;  // just the hz lines, because there's room. vt highlight lines are just +1

void SetLinePixel(int row, int col, eInkDisplay::Colour colour = CFG_CAL_LINES_COLOUR, bool solid = kSolidLines)
{
  // Sets a line pixel at (current) row and col, alternating if !solid
#ifdef CFG_EINK_UPRIGHT_DISPLAY
  col = eInkDisplay::WIDTH - col - 1;
#endif
  if (solid || !((row + col + kBump) % 2)) // for current numbers, adding the <not> sets corner pixels ON
    eInkDisplay::SetRowBufferAt(col, colour);
}

int _DaysThisMonth = 0;
int _DaysPrevMonth = 0;
int _DaysCounter = 0;
int DateAt(int CellRow, int CellCol)
{
  // Returns the date to show in the cell, -ve if not the current month
  // For example, the first week in April 2026 is
  // |Sun|Mon|Tue|Wed|Thu|Fri|Sat|
  // |-29|-30|-31| 1 | 2 | 3 | 4 |
  // etc.  -ve dates are pre/next month, grey/omitted
  int DaysCounter = _DaysCounter + (CellRow - 2)*kDaysPerWeek + CellCol;
  if (DaysCounter <= 0)
    return -(_DaysPrevMonth + DaysCounter); // prev month
  else if (DaysCounter > _DaysThisMonth)
    return -(DaysCounter - _DaysThisMonth); // next month
  return DaysCounter;                       // this month
}

// "Www dd Mmm 20YY":
char pFullDateBuffer[32]; 
void InsertText(int row, int CellRow)
{
  // Injects rows of text pixels into the buffer
  // Row-by-row. Not very efficient, but it doesn't really matter in this context
  char buff[32];
  if (!ATC::_TimeSet)
  {
    eInkText::InsertText(eInkDisplay::rowBuffer, row, kOffsetTop + kCellTextOffsetRow + kCellHeight*0, eInkDisplay::WIDTH/2, Config::pSplashLines0, eInkDisplay::MonoBlack, kScale, true);
    eInkText::InsertText(eInkDisplay::rowBuffer, row, kOffsetTop + kCellTextOffsetRow + kCellHeight*1, eInkDisplay::WIDTH/2, Config::pSplashLines1, eInkDisplay::MonoBlack, kScale, true);
    eInkText::InsertText(eInkDisplay::rowBuffer, row, kOffsetTop + kCellTextOffsetRow + kCellHeight*4, eInkDisplay::WIDTH/2, Config::pNoConfigStr,  eInkDisplay::MonoBlack, kScale, true, true, false);
    return;
  }

  if (CellRow < 0)
    return;

  int atRow = kOffsetTop + CellRow*kCellHeight + kCellTextOffsetRow;
  if (CellRow == 0) // Date
    eInkText::InsertText(eInkDisplay::rowBuffer, row, kOffsetTop + kCellTextOffsetRow, kOffsetLeft + kCellTextOffsetCol3, pFullDateBuffer, eInkDisplay::MonoBlack, kScale, false, false, true);
  else if (CellRow == 1)  // Days of week
    for (int col = 0; col < kCellCols; col++)
    {
      ::strcpy_P(buff, MSTR_StrN(Config::pDayShortNames, (col + CFG_CAL_FIRST_WEEKDAY) % kDaysPerWeek));
      buff[2] = 0; // truncate to 2 chars
      eInkText::InsertText(eInkDisplay::rowBuffer, row, atRow, kOffsetLeft + col * kCellWidth + kCellTextOffsetCol2, buff, eInkDisplay::MonoBlack, kScale, false, false, true);
    }
  else // Dates
  {
    for (int day = 0; day < 7; day++)
    {
      int date = DateAt(CellRow, day);
      eInkDisplay::Colour colour = eInkDisplay::MonoBlack;
      if (date < 0)
        colour = CFG_CAL_OTHER_MONTH_COLOUR;
      date = abs(date);
      ::itoa(date, buff, 10);
      eInkText::InsertText(eInkDisplay::rowBuffer, row, atRow, kOffsetLeft + day*kCellWidth + ((date<10) ? kCellTextOffsetCol1 : kCellTextOffsetCol2), buff, colour, kScale, false, false, true);
    }
  }
}

void DrawCalendar()
{
  // Draw calendar page
  int Year = ATC::_Year;
  int Month = ATC::_Month;
  int Date = ATC::_Date;
  int DayOfWeek = ATC::_DayOfWeek;
  
  // Build the date string "Www dd Mmm 20YY"
  ::strcpy_P(pFullDateBuffer, MSTR_StrN(Config::pDayShortNames, DayOfWeek));
  ::strcat_P(pFullDateBuffer, Config::pDateExtras);
  ::itoa(Date, pFullDateBuffer + strlen(pFullDateBuffer), 10);
  ::strcat_P(pFullDateBuffer, Config::pDateExtras);
  size_t pos = ::strlen(pFullDateBuffer);
  ::strcat_P(pFullDateBuffer, MSTR_StrN(Config::pMonthLongNames, Month - 1));
  pFullDateBuffer[pos + 3] = 0; // truncate to 3 chars
  ::strcat_P(pFullDateBuffer, Config::pDateExtras);
  ::itoa(2000 + Year, pFullDateBuffer + strlen(pFullDateBuffer), 10);
  
  // Calc calendar days
  _DaysThisMonth = DaysInMonth(Month, Year);
  _DaysPrevMonth = DaysInMonth((Month == 1) ? 12 : Month - 1, (Month == 1) ? Year - 1 : Year);
  _DaysCounter = Date - DayOfWeek + CFG_CAL_FIRST_WEEKDAY;  // start of this week
  // Go to the start, "date" of the top-left calendar cell:
  while (_DaysCounter > 1)
    _DaysCounter -= kDaysPerWeek; // previous weeks

  eInkDisplay::StartMono();
  for (int r = 0; r < eInkDisplay::HEIGHT; r++)
  {
    int row = r;
    #ifdef CFG_EINK_UPRIGHT_DISPLAY
      row = eInkDisplay::HEIGHT - r - 1;
    #endif
    eInkDisplay::FillRowBuffer(eInkDisplay::MonoWhite);
    if (row >= kOffsetTop && row < kOffsetTop + kColHeight)
    {
      int CellRow = (row - kOffsetTop) / kCellHeight;
      int RowMod = (row - kOffsetTop) % kCellHeight;
      bool DoubleLine = CellRow == 1 && RowMod == kCellHeight - 1;  // thicker line between day names and dates
      if (RowMod == 0 || DoubleLine)  // horizontal lines
        for (int i = 0; i < kRowWidth; i++)
          SetLinePixel(row, kOffsetLeft + i);
      else                            // vertical lines
        if (CellRow)
        {
          for (int c = kOffsetLeft; c < eInkDisplay::WIDTH - kOffsetLeft; c++)
          {
            int col = c;
            #ifdef CFG_EINK_UPRIGHT_DISPLAY
            col = eInkDisplay::WIDTH - c - 1;
            #endif
            int CellCol = (col - kOffsetLeft) / kCellWidth;
            bool highlight = (CellRow >= 2) && (CellCol < kDaysPerWeek) && DateAt(CellRow, CellCol) == Date && ATC::_TimeSet;

            if (((col - kOffsetLeft) % kCellWidth) == 0)
            {
              // col falls on a vertical, set the pixel
              SetLinePixel(row, col);
              if (highlight)
              {
                // thicken Today's walls
                SetLinePixel(row, col + 1, eInkDisplay::MonoBlack, true);
                SetLinePixel(row, col + kCellWidth - 1, eInkDisplay::MonoBlack, true);
              }
            }
            else if (highlight && (RowMod <= kCellHighlightThickness || RowMod >= (kCellHeight - kCellHighlightThickness)))
              SetLinePixel(row, col, eInkDisplay::MonoBlack, true); // thicken top & bottom of Today cell
          }
        }
        else
        {
          // top row of cells is open, for the full date
          SetLinePixel(row, kOffsetLeft);
          SetLinePixel(row, kOffsetLeft + kRowWidth - 1);
        }
      InsertText(r, CellRow);
    }
    eInkDisplay::SendRowBuffer();
  }
}

const char pStr_ST[] PROGMEM = "st";
const char pStr_ND[] PROGMEM = "nd";
const char pStr_RD[] PROGMEM = "rd";
const char pStr_TH[] PROGMEM = "th";
void DrawDate()
{
  // Draw just the date:
  //    Sat
  //     6
  //    Jun
  int Month = ATC::_Month;
  int Date = ATC::_Date;
  int DayOfWeek = ATC::_DayOfWeek;
  char pDateBuff[10];
  ::itoa(Date, pDateBuff, 10);
  // Append ordinal indicator? 
#if 0 // Nah - keep it simple  
  int DateMod = Date % 10;
  if (10 <= Date && Date <= 19)
    ::strcat_P(pDateBuff, pStr_TH);
  else if (DateMod == 1)
    ::strcat_P(pDateBuff, pStr_ST);
  else if (DateMod == 2)
    ::strcat_P(pDateBuff, pStr_ND);
  else if (DateMod == 3)
    ::strcat_P(pDateBuff, pStr_RD);
  else
    ::strcat_P(pDateBuff, pStr_TH);
#endif

  ::strcpy_P(pFullDateBuffer, MSTR_StrN(Config::pMonthLongNames, Month - 1));
  pFullDateBuffer[3] = 0;

  eInkDisplay::StartMono();
  for (int r = 0; r < eInkDisplay::HEIGHT; r++)
  {
    eInkDisplay::FillRowBuffer(eInkDisplay::MonoWhite);
    if (!ATC::_TimeSet)
      InsertText(r, -1);
    else
    {
#ifdef CFG_EINK_8x8_FONT
      const int kScale = 6;
      const int kFontHeight = 8;
#else
      const int kScale = 7;
      const int kFontHeight = 6;
#endif
      const int kLineHeight = kScale*kFontHeight;
      const int kLineGap = kScale*2;
      int AtRow = (eInkDisplay::HEIGHT - 3*kLineHeight - 2*kLineGap)/2;
      eInkText::InsertText(eInkDisplay::rowBuffer, r, AtRow, eInkDisplay::WIDTH/2, MSTR_StrN(Config::pDayShortNames, DayOfWeek), eInkDisplay::MonoBlack, kScale, true, false, false);
      AtRow += kLineHeight + kLineGap;
      eInkText::InsertText(eInkDisplay::rowBuffer, r, AtRow, eInkDisplay::WIDTH/2, pDateBuff, eInkDisplay::MonoBlack, kScale, false, false, false);
      AtRow += kLineHeight + kLineGap;
      eInkText::InsertText(eInkDisplay::rowBuffer, r, AtRow, eInkDisplay::WIDTH/2, pFullDateBuffer, eInkDisplay::MonoBlack, kScale, false, false, false);
    }
    eInkDisplay::SendRowBuffer();
  }
}

void Draw(bool full)
{
  // No room for a full screen buffer so rendered line-by-line, like the moon
  OLED::On(true);
  OLEDText::DisplayLine(Config::pBusyStr, 0, true);
  OLEDText::DisplayLine(Config::pEmptyStr, 1, true);
  
  if (full)
    DrawCalendar();
  else
    DrawDate();

  eInkDisplay::StartRed();
  eInkDisplay::FillRowBuffer(eInkDisplay::ColourNone);
  for (int row = 0; row < eInkDisplay::HEIGHT; row++)
    eInkDisplay::SendRowBuffer(eInkDisplay::rowBuffer);

  eInkDisplay::Refresh();
  OLED::On(false);
}

}
