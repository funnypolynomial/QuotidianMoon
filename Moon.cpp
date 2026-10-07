#include <Arduino.h>
#include "ATC.h"
#include "Calendar.h"
#include "Config.h"
#include "eInkDisplay.h"
#include "eInkText.h"
#include "Moon.h"
#include "MoonData.h"
#include "OLED.h"
#include "OLEDText.h"

namespace Moon
{
#ifdef CFG_MOON_NORTHERN_HEMISPHERE
  #ifdef CFG_EINK_UPRIGHT_DISPLAY
    const uint8_t* pMoonImageRows = pMoonImageRowsSouth;
  #else
    const uint8_t* pMoonImageRows = pMoonImageRowsNorth;
  #endif
#else
  #ifdef CFG_EINK_UPRIGHT_DISPLAY
    const uint8_t* pMoonImageRows = pMoonImageRowsNorth;
  #else
    const uint8_t* pMoonImageRows = pMoonImageRowsSouth;
  #endif
#endif

// halftone patterns
#define W eInkDisplay::MonoWhite
#define G eInkDisplay::MonoGrey
#define B eInkDisplay::MonoBlack
static const eInkDisplay::Colour palette[16][4] PROGMEM =
{
   // a, b
   // c, d
  {B,B,
   B,B},

  {B,B,
   B,G},
      {B,B,
       B,G},
  {G,B,
   B,G},
      {G,B,
       B,G},
  {G,G,
   B,G},
      {G,G,
       B,G},
  {G,G,
   G,G},
      {G,G,
       G,G},
  {G,G,
   G,W},
      {G,G,
       G,W},
  {W,G,
   G,W},
      {W,G,
       G,W},
  {W,W,
   G,W},
      {W,W,
       G,W},

  {W,W,
   W,W},

};
#undef W
#undef G
#undef B

// Bhaskara I's sine approximation:
long rcosx(long r, long x)
{
  x = 90 - x;
  if (x < 0) 
  {
    x += 180;
    r = -r;
  }
  if (x > 180)
  {
    x -= 180;
    r = -r;
  }
  return r*4L*x*(180L-x)/(40500L - x*(180L-x));
}

int _moonAge = -1;
// Moon image pixels are 2x2 physical pixels on the display, we're drawing
// two rows at a time using the eInkDisplay buffer, and this:
byte _upperRowBuffer[eInkDisplay::WIDTH/4]; // the upper row in the pair, 2 bpp

bool HaveNoConfig()
{
  return Config::_NewMoonReferenceSeconds == 0 || !ATC::_TimeSet;
}

const int kScale = 2;

void SendRow(int& row, uint8_t* pRowBuffer)
{
  // Send a row to the display after inserting text pixels
#ifdef CFG_MOON_TEXT_COLOUR
  if (!HaveNoConfig())
  {
    // Date etc
    // WWW      DD
    //
    // MMM     NNd
    char buff[16];
    //                                    row, col
    ::strcpy_P(buff, MSTR_StrN(Config::pDayShortNames, ATC::_DayOfWeek));
    eInkText::InsertText(pRowBuffer, row, +2,   +2, buff, CFG_MOON_TEXT_COLOUR, kScale, false);   // day of week
    eInkText::InsertText(pRowBuffer, row, +2,   -3, ::itoa(ATC::_Date, buff, 10), CFG_MOON_TEXT_COLOUR, kScale, false);     // date
    ::strcpy_P(buff, MSTR_StrN(Config::pMonthLongNames, ATC::_Month - 1)); // month
    buff[3] = 0; // truncate
    eInkText::InsertText(pRowBuffer, row, -2,   +2, buff, CFG_MOON_TEXT_COLOUR, kScale, false); // month
#ifdef CFG_MOON_IS_APPROX
    // year
    buff[0] =  '\'';
    ::itoa(ATC::_Year, buff + 1, 10);
#else
    // moon age
    if (_moonAge < 0)
      ::strcpy(buff, "?");
    else
      ::itoa(_moonAge, buff, 10);
    ::strcat(buff, "d");
#endif    
    eInkText::InsertText(pRowBuffer, row, -2,   -3, buff, CFG_MOON_TEXT_COLOUR, kScale, false);
  }
#endif

  if (HaveNoConfig())
  {
    eInkText::InsertText(pRowBuffer, row, eInkDisplay::HEIGHT/6, eInkDisplay::WIDTH/2, Config::pSplashLines0, eInkDisplay::MonoBlack, kScale, true);
    eInkText::InsertText(pRowBuffer, row, eInkDisplay::HEIGHT/6 + eInkText::TextHeight(kScale) + 2, eInkDisplay::WIDTH/2, Config::pSplashLines1, eInkDisplay::MonoBlack, kScale, true);
    eInkText::InsertText(pRowBuffer, row, eInkDisplay::HEIGHT/2, eInkDisplay::WIDTH/2, Config::pNoConfigStr, eInkDisplay::MonoBlack, kScale, true, true);
  }
  row++;
  eInkDisplay::SendRowBuffer(pRowBuffer);
}

void DrawGore(int startAngle, int endAngle)
{
  // Draws a "gore" of the moon image, a strip of pixels between the two meridian angles, 0 starting from the LEFT. 180 on the RIGHT
  // We're doubling the size of the image, to give a 2x2 "pixel" and more palette entries
  const int scale = 2;
  const uint8_t* ptr = pMoonImageRows;
  int left = (eInkDisplay::WIDTH/scale - MOON_IMAGE_WIDTH)/2;
  int top = (eInkDisplay::HEIGHT/scale - MOON_IMAGE_HEIGHT)/2;
  uint8_t startEdges[MOON_IMAGE_WIDTH];
  uint8_t endEdges[MOON_IMAGE_WIDTH];
  bool greyDisk = false;
#ifdef CFG_MOON_GREY_DISK
  if (abs(endAngle - startAngle) < CFG_MOON_GREY_DISK)
  {
    // force a very thin (or no) illuminated sliver to be a grey disk
    greyDisk = true;
    startAngle = 0;
    endAngle = 180;
  }
#endif
  // pre-build the trig calculations so they don't happen while we're drawing, a delay before we start is better than while painting a strip (N/A with eInk!)
  for (int i = 0; i <=  MOON_IMAGE_WIDTH/2; i++)
  {
    startEdges[i] = i + rcosx(i, startAngle);
    endEdges[i] = i + rcosx(i, endAngle);
  }
  eInkDisplay::StartMono();
  int displayRow = 0;
  // upper blank rows
  eInkDisplay::FillRowBuffer(eInkDisplay::MonoBlack);
  for (int r = 0; r < scale*top; r++)
    SendRow(displayRow, eInkDisplay::rowBuffer);
  // the data is a list of rows with # black pixels first then two-pixels per byte image detail
  // we need to skip over data up to the point we start drawing, draw the pixels then skip the rest of the row
  // to arrive at the start of the next
  for (int row = 0; row < MOON_IMAGE_HEIGHT; row++)
  {
    int blackCols = pgm_read_byte(ptr++);
    int colourCols = MOON_IMAGE_WIDTH - 2*blackCols;
    int startEdge = startEdges[colourCols/2];
    int endEdge = endEdges[colourCols/2];
    eInkDisplay::FillRowBuffer(_upperRowBuffer, eInkDisplay::MonoBlack);
    eInkDisplay::FillRowBuffer(eInkDisplay::MonoBlack);
    eInkDisplay::StartRowBufferWrite(scale*(left + blackCols + colourCols - startEdge));
    // skip whole leading bytes
    while ((colourCols - startEdge) > 2)
    {
      ptr++;
      colourCols -= 2;
    }
    while (colourCols > 0)
    {
      uint8_t pair = pgm_read_byte(ptr++);
      for (int pix = 0; pix < 2; pix++)
      {
        if (colourCols-- > 0)
        {
          if (colourCols >= endEdge)
          {
            const eInkDisplay::Colour* pColour = palette[pair & 0x0F];
            if (HaveNoConfig())
              pColour = palette[0x0F];  // just draw the disk
            else if (greyDisk)
              pColour = palette[0x01];  // (full) faint grey disk
            // ab
            // cd
            if (eInkDisplay::GetRowBufferWriteCol() < (eInkDisplay::WIDTH - 2))
            {
              eInkDisplay::SetRowBufferAt(_upperRowBuffer, eInkDisplay::GetRowBufferWriteCol(), (eInkDisplay::Colour)pgm_read_byte(pColour++));
              eInkDisplay::SetRowBufferAt(_upperRowBuffer, eInkDisplay::GetRowBufferWriteCol() + 1, (eInkDisplay::Colour)pgm_read_byte(pColour++));
              eInkDisplay::WriteRowBuffer((eInkDisplay::Colour)pgm_read_byte(pColour++));
              eInkDisplay::WriteRowBuffer((eInkDisplay::Colour)pgm_read_byte(pColour++));
            }
          }
          else if (colourCols < endEdge)
          {
            // past the last pixel, skip the remaining bytes
            while (colourCols > 0)
            {
              ptr++;
              colourCols -= 2;
            }
            pix = 2;
          }
        }
        pair >>= 4;
      }
    }
    // send the pair of rows
    SendRow(displayRow, _upperRowBuffer);
    SendRow(displayRow, eInkDisplay::rowBuffer);
  }
  // lower blank rows
  eInkDisplay::FillRowBuffer(eInkDisplay::MonoBlack);
  for (int r = scale*(MOON_IMAGE_HEIGHT + top); r < eInkDisplay::HEIGHT; r++)
    SendRow(displayRow, eInkDisplay::rowBuffer);

  // clear the red
  eInkDisplay::StartRed();
  eInkDisplay::FillRowBuffer(eInkDisplay::ColourNone);
  for (int row = 0; row < eInkDisplay::HEIGHT; row++)
    eInkDisplay::SendRowBuffer(eInkDisplay::rowBuffer);
  
  eInkDisplay::Refresh();
}

void DrawPhase(int phaseAngle)
{
  // Draw the moon rotated by the phase angle
  OLED::On(true);
  OLEDText::DisplayLine(Config::pBusyStr, 0, true);
  OLEDText::DisplayLine(Config::pEmptyStr, 1, true);
  // complex inversions for hemisphere and orientation
#ifdef CFG_MOON_NORTHERN_HEMISPHERE
  #ifdef CFG_EINK_UPRIGHT_DISPLAY
    if (phaseAngle <= 90)
      DrawGore(0, 2*phaseAngle);
    else
      DrawGore(2*(phaseAngle - 90), 180);
  #else
    if (phaseAngle <= 90)
      DrawGore(180 - 2*phaseAngle, 180);
    else
      DrawGore(0, 180 - 2*(phaseAngle - 90));
  #endif
#else  // Southern
  #ifdef CFG_EINK_UPRIGHT_DISPLAY
    if (phaseAngle <= 90)
      DrawGore(180 - 2*phaseAngle, 180);
    else
      DrawGore(0, 2*(180 - phaseAngle));
  #else
    if (phaseAngle <= 90)
      DrawGore(0, 2*phaseAngle);
    else
      DrawGore(2*(phaseAngle - 90), 180);
  #endif
#endif
  OLEDText::DisplayLine(NULL, 0, false);
  OLED::On(false);
}

void Draw()
{
  // Draw moon at current phase
  if (HaveNoConfig())
  {
    _moonAge = -1;
    Moon::DrawPhase(90); // just draw the disk
  }
  else
  {
    int phase = CalcPhase(ATC::_Year, ATC::_Month, ATC::_Date, _moonAge);
    Moon::DrawPhase(phase); // 0=new 90=full
  }
}

const uint32_t kMoonPeriodSeconds = 2551443UL;  // ~29.530587981 days
uint32_t CalcReference(uint8_t days, uint8_t hour, uint8_t minute)
{
  // Calculate the date-time of the new moon
  uint32_t ref = 0;
  // Reference for the START of TODAY
  ref += Calendar::SecondsSinceReference(ATC::_Year, ATC::_Month, ATC::_Date, 0, 0);
  // Plus days in the future. 1 day is tomorrow, so add all of today
  ref += Calendar::kSecondsPerDay*days;
  // Plus time in the new moon day
  ref += 60UL*Calendar::TimeToMinutes(hour, minute);
  return ref;
}

int CalcPhase(int Year, int Month, int Date, int& Age)
{
  // Compute the Moon Age (days since New) and return the phase angle
  // Reference for noon of the day
  uint32_t nowSeconds = Calendar::SecondsSinceReference(Year, Month, Date, 12, 0);
  // Seconds since the reference New moon to now
  uint32_t diffSeconds = nowSeconds - Config::_NewMoonReferenceSeconds;
  if (nowSeconds < Config::_NewMoonReferenceSeconds) // we are before the New Moon
    diffSeconds = kMoonPeriodSeconds - (Config::_NewMoonReferenceSeconds - nowSeconds);
  // Age is the remainder divided by the period
  uint32_t ageSeconds = diffSeconds % kMoonPeriodSeconds;
  // Days since New
  uint32_t ageDays = ageSeconds / Calendar::kSecondsPerDay;
  if ((ageSeconds % Calendar::kSecondsPerDay) > Calendar::kSecondsPerDay / 2)  // round up
    ageDays++;
  // Phase angle
  uint32_t angle = (ageSeconds * 180UL) / kMoonPeriodSeconds;
  if (((ageSeconds * 180UL) % kMoonPeriodSeconds) > kMoonPeriodSeconds / 2)  // round up
    angle++;
  Age = (int)ageDays;
  return (int)angle;
}

void CalcNextNewMoon(int Year, int Month, int Date, int& days, int& hour, int& minute)
{
  // Compute the number of days, and the time, of the next new moon
  days = hour = minute = 0;
  if (Config::_NewMoonReferenceSeconds)
  {
    // Start of today
    uint32_t todaySeconds = Calendar::SecondsSinceReference(Year, Month, Date, 0, 0);
    // Next New Moon
    uint32_t NextNewAtSeconds = Config::_NewMoonReferenceSeconds;
    while (todaySeconds > NextNewAtSeconds)
      NextNewAtSeconds += kMoonPeriodSeconds;
    // Seconds until next new moon
    uint32_t diffSeconds = NextNewAtSeconds - todaySeconds;
    uint32_t wholeDays = diffSeconds / Calendar::kSecondsPerDay;
    uint32_t seconds = diffSeconds % Calendar::kSecondsPerDay;
    days = (int)wholeDays;
    Calendar::MinutesToTime(seconds/60UL, hour, minute);
  }
}

const char pPhaseLines0[] PROGMEM  = MSTR("New")  MSTR("Waxing")   MSTR("First")   MSTR("Waxing")  MSTR("Full") MSTR("Waning")  MSTR("Third/Last") MSTR("Waning");
const char pPhaseLines1[] PROGMEM  = MSTR("Moon") MSTR("Crescent") MSTR("Quarter") MSTR("Gibbous") MSTR("Moon") MSTR("Gibbous") MSTR("Quarter")    MSTR("Crescent");
void GetPhaseName(int phase, const char*& Line0, const char*& Line1)
{
  // Return name of the phase
  // do "n=phase/22.5" but with ints
  phase *= 2;
  int n = phase/45;
  if ((phase % 45) >= 23)
    n++;
  n %= 8;
  Line0 = MSTR_StrN(pPhaseLines0, n);
  Line1 = MSTR_StrN(pPhaseLines1, n);
}
}
