#include <Arduino.h>
#include <EEPROM.h>
#include "ABTN.h"
#include "ATC.h"
#include "Calendar.h"
#include "Config.h"
#include "eInkDisplay.h"
#include "Monitor.h"
#include "Moon.h"
#include "OLED.h"
#include "OLEDText.h"
#include "SplashData.h"

// Extract the nth string from a multi-string
const char* MSTR_StrN(const char* multiStr, uint8_t n)
{
  // return the Nth string in the given mult-string, or an empty string
  // the first is 0
  while (n && pgm_read_byte(multiStr))
  {
    while (pgm_read_byte(multiStr))
      multiStr++;
    multiStr++;
    n--;
  }
  return pgm_read_byte(multiStr)?multiStr:(Config::pEmptyStr);
}

namespace Config
{
const int kStepMinutes = 5; // set time to nearest 5
const int kEEPROMSigIdx = 0; // idx of first byte of config block
const uint32_t kEEPROMSignature = 0x51754D6FUL;  // "QuMo" // DWORD at EEPROM[0] to show values are present
const uint32_t kBlinkIntervalMS = 500UL;    // blink value 0.5s
const uint32_t kIdleIntervalMS  = 30000UL;  // bail if idle 30s
const uint32_t kButtonHeldMS    = 3000UL;   // hold duration 3s
uint32_t _NewMoonReferenceSeconds = 0;
bool     _MonitorOn = true;
uint8_t _Face = 0;  // 0/1/2 = Moon/Calendar/Date

char LineBuffer[OLEDText::kCharsPerLine + 1];

const char pConfigure[] PROGMEM = "Configure";
#ifdef CFG_TIME_USE_DLS
enum tTopMenu {eDLS, eTime, eDate, eMoon, eAdjust, eScrub, eExit};
const char pConfigureItems[] PROGMEM = MSTR("DLS") MSTR("Time") MSTR("Date") MSTR("Moon") MSTR("Adjust") MSTR("Scrub eInk") MSTR("Exit");
#else
enum tTopMenu {eTime, eDate, eMoon, eAdjust, eScrub, eExit};
const char pConfigureItems[] PROGMEM = MSTR("Time") MSTR("Date") MSTR("Moon") MSTR("Adjust") MSTR("Scrub") MSTR("Exit");
#endif
const char pOffOnItems[] PROGMEM  = MSTR("Off") MSTR("On");
const char pTimePrompts[] PROGMEM = MSTR("Hour") MSTR("Minute");
const char pDatePrompts[] PROGMEM = MSTR("Year") MSTR("Month") MSTR("Date") MSTR("Day");
const char pMonthLongNames[] PROGMEM = MSTR("January") MSTR("February") MSTR("March") MSTR("April") MSTR("May") MSTR("June") MSTR("July") MSTR("August") MSTR("September") MSTR("October") MSTR("November") MSTR("December");
const char pDayShortNames[] PROGMEM  = MSTR("Sun") MSTR("Mon") MSTR("Tue") MSTR("Wed") MSTR("Thu") MSTR("Fri") MSTR("Sat");
const char pMoonPrompts[] PROGMEM    = MSTR("New in") MSTR("New at hr")  MSTR("New at min");
const char pDaysSuffix[] PROGMEM = MSTR(" days");
const char pYearPrefix[] PROGMEM = MSTR("'");
const char pDateExtras[] PROGMEM = MSTR(" ") MSTR(" '") MSTR(":") MSTR("S") MSTR("D") MSTR("??:??");
const char pSplashLines0[] PROGMEM  = MSTR("Quotidian") MSTR("MEW fecit") MSTR("SET:Config") MSTR("SEL:Face");
const char pSplashLines1[] PROGMEM  = MSTR("Moon")      MSTR("MMXXVI")    MSTR("Held:View")  MSTR("Held:Skip");
const char pSetHeldLines[] PROGMEM = MSTR("SET:Exit") MSTR("SEL:Next");
const char pMoonLines[] PROGMEM = MSTR("New@ ") MSTR("In ") MSTR("Phase");
const char pLDRStrings[] PROGMEM = MSTR("LDR  (") MSTR(")") MSTR("=Day") MSTR("=Night");
const char pAdjustStrings[] PROGMEM = MSTR("Adj@ ") MSTR("+") MSTR("-") MSTR("m") MSTR("None") MSTR("Skip");
const char pNoConfigStr[] PROGMEM = "NO CONFIG";
const char pBusyStr[] PROGMEM = "busy";
const char pMonitorStr[] PROGMEM = "Monitor";
const char pSkipAdjStr[] PROGMEM = MSTR("(Skipping") MSTR("(Doing") MSTR("next adj)");
const char pEmptyStr[] PROGMEM = "";

bool Menu(const char* pPrompt, uint8_t& value, uint8_t min, uint8_t max, const char* pValueMSTR, uint8_t inc = 1, void (*pCallback)(uint8_t) = NULL)
{
  // Make a selection
  // pPrompt on line 0, value on line 1. value in the range. 
  // Value incremented by inc when Sel hit, accepted when Set hit
  // if pValueMSTR is not null, value is pValueMSTR[value - min], otherwise the integer.
  // if there's just one string in pValueMSTR, it as used as a suffix to the integer, or a prefix if it's 1 char!
  OLEDText::DisplayLine(pPrompt, 0, true);
  bool update = true;
  bool blink = false;
  bool suffix = pValueMSTR && MSTR_StrN(pValueMSTR, 1) == pEmptyStr;
  uint32_t blinkTimerMS = millis();
  uint32_t idleTimerMS = millis();
  if (value < min || value > max)
    value = min;  // likely only applies with date after month changed
  while (true)
  {
    if (update)
    {
      // redraw the value
      if (pValueMSTR && !suffix) // string
        ::strcpy_P(LineBuffer, MSTR_StrN(pValueMSTR, value - min));
      else
      {
        // int
        char* pBuff = LineBuffer;
        if (suffix && strlen_P(pValueMSTR) == 1)    // prefix
          ::strcpy_P(pBuff++, pValueMSTR);
        if (min == 0 && !pValueMSTR && value < 10) // show leading 0
          *pBuff++ = '0';
        ::itoa(value, pBuff, 10);
        if (suffix && strlen_P(pValueMSTR) > 1)
          ::strcat_P(pBuff, pValueMSTR);
      }
      if (pCallback)
        pCallback(value);
      OLEDText::DisplayLine(blink?NULL:LineBuffer, 1, false);
      update = false;
    }
    ABTN::tButton btn = btns.Pressed();
    if (btn == ABTN::eSet) // accepted, get out
      return true;
    else if (btn == ABTN::eSel)
    {
      // increment
      value += inc;
      if (value > max)  // wrap
        value = min;
      update = true;
      blink = false;
      idleTimerMS = millis();
    }
    else
    {
      uint32_t nowMS = millis();
      if (nowMS - blinkTimerMS > kBlinkIntervalMS) // blink timer
      {
        blink = !blink;
        blinkTimerMS = millis();
        update = true;
      }
      else if (nowMS - idleTimerMS > kIdleIntervalMS)  // idle timer
        return false;
    }
  }
  return false;
}

void Save()
{
  // Save Moon reference, DLS & Date etc to EEPROM
  int idx = kEEPROMSigIdx;
  EEPROM.put(idx, kEEPROMSignature);
  idx += sizeof(kEEPROMSignature);
  EEPROM.put(idx, _NewMoonReferenceSeconds);
  idx += sizeof(_NewMoonReferenceSeconds);
  EEPROM.put(idx++, ATC::_DLS);
  EEPROM.put(idx++, ATC::_Year);
  EEPROM.put(idx++, ATC::_Month);
  EEPROM.put(idx++, ATC::_Date);
  EEPROM.put(idx++, ATC::_DayOfWeek);
  EEPROM.put(idx++, _Face);
  EEPROM.put(idx++, _MonitorOn);
}

void Load()
{
  // Load Moon reference, DLS & Date etc from EEPROM
  int idx = kEEPROMSigIdx;
  uint32_t sig = 0;
  EEPROM.get(idx, sig);
  if (sig == kEEPROMSignature)
  {
    idx += sizeof(kEEPROMSignature);
    EEPROM.get(idx, _NewMoonReferenceSeconds);
    idx += sizeof(_NewMoonReferenceSeconds);
    ATC::_DLS        = EEPROM.read(idx++);
    ATC::_Year       = EEPROM.read(idx++);
    ATC::_Month      = EEPROM.read(idx++);
    ATC::_Date       = EEPROM.read(idx++);
    ATC::_DayOfWeek  = EEPROM.read(idx++);
    _Face            = EEPROM.read(idx++);
    _MonitorOn       = EEPROM.read(idx++);
  }
  // else fall back to __DATE__
  
#ifndef CFG_TIME_USE_DLS  
  ATC::_DLS = 0;
#endif
}

bool ConfigureTime()
{
  // Configure Hour (24) and Minute (5's)
  uint8_t hour = ATC::GetHour24(ATC::_Hour24);
  uint8_t minute = ATC::_Minute/kStepMinutes;
  minute *= kStepMinutes;
  if (Menu(pTimePrompts, hour, 0, 23, NULL))
    if (Menu(MSTR_StrN(pTimePrompts, 1), minute, 0, 60 - kStepMinutes, NULL, kStepMinutes))
    {
      ATC::Set(hour, minute);
      return true;
    }
  return false;
}

uint8_t year;
uint8_t month;
uint8_t firstDay;
void DayOfWeekCallback(uint8_t date)
{
  // Callback to append computed day of week to the date
  ::strcat_P(LineBuffer, pDateExtras);
  ::strcat_P(LineBuffer, MSTR_StrN(pDayShortNames, (firstDay + date - 1) % Calendar::kDaysPerWeek));
}

bool ConfigureDate()
{
  // Configure Decade, Month, Date and Day of week (Sun = 0)
  year = ATC::_Year;
  month = ATC::_Month;
  uint8_t date = ATC::_Date;
  if (Menu(pDatePrompts, year, 26, 50, pYearPrefix)) // max=2050 is optimistic!
    if (Menu(MSTR_StrN(pDatePrompts, 1), month, 1, 12, pMonthLongNames))
    {
      firstDay = Calendar::DayOfWeek(year, month, 1);
      if (Menu(MSTR_StrN(pDatePrompts, 2), date, 1, Calendar::DaysInMonth(month, year), NULL, 1, DayOfWeekCallback))
      {
        ATC::_Year = year;
        ATC::_Month = month;
        ATC::_Date = date;
        ATC::_DayOfWeek = Calendar::DayOfWeek(ATC::_Year, ATC::_Month, ATC::_Date);
        Save();
        return true;
      }
    }
  return false;
}

bool ConfigureMoon()
{
  // Configure next new moon. In nn days, as hh:mm
  uint8_t days = 1;
  uint8_t hour = 12;
  uint8_t minute = 0;
  if (_NewMoonReferenceSeconds && ATC::_TimeSet)
  {
    int d, h, m;
    Moon::CalcNextNewMoon(ATC::_Year, ATC::_Month, ATC::_Date, d, h, m);
    days = d;
#ifndef CFG_MOON_IS_APPROX
    hour = h;
    minute = m/kStepMinutes;
    minute *= kStepMinutes;
#endif    
  }
  if (Menu(pMoonPrompts, days, 0, 29, pDaysSuffix))
    if (Menu(MSTR_StrN(pMoonPrompts, 1), hour, 0, 23, NULL))
      if (Menu(MSTR_StrN(pMoonPrompts, 2), minute, 0, 60 - kStepMinutes, NULL, kStepMinutes))
      {
        _NewMoonReferenceSeconds = Moon::CalcReference(days, ATC::GetHour24(hour), minute);
        Save();
        return true;
      }
  return false;
}

//                                          01234567890
const char pDATE[] PROGMEM  = __DATE__; // "Mmm dd yyyy"
void LoadBuildDate()
{
  // init the UDC date from __DATE__
  int temp;
  char buff[32];
  strcpy_P(buff, pDATE);
  // month, compare 1st & 3rd chars
  for (temp = 0; temp < 12; temp++)
  {
    const char* pMonth = MSTR_StrN(pMonthLongNames, temp);
    if (*buff       == pgm_read_byte(pMonth) &&
        *(buff + 2) == pgm_read_byte(pMonth + 2))
      ATC::_Month = temp + 1;
  }
  // date
  temp = ::atoi(buff + 4);
  if (1 <= temp && temp <= 32)
    ATC::_Date = temp;
  // year
  temp = ::atoi(buff + 7);
  if (2026 <= temp && temp <= 2050)
    ATC::_Year = temp - 2000;
  // day
  ATC::_DayOfWeek = Calendar::DayOfWeek(ATC::_Year, ATC::_Month, ATC::_Date);
}

const uint32_t kTextLinger = 2000UL; // how long the text lingers
void SplashImage()
{
  // draw the moon phases splash image
  uint8_t pageBuffer[OLED::WIDTH];
#ifdef CFG_MOON_NORTHERN_HEMISPHERE  
  const uint8_t* ptr = pSplashImageDataNorth;
#else  
  const uint8_t* ptr = pSplashImageDataSouth;
#endif  
  bool black = true;
  int ctr = pgm_read_byte(ptr++);
  for (int page = 0; page < 2; page++)
  {
    ::memset(pageBuffer, 0, sizeof(pageBuffer));
    // extract the encoded rows
    for (int bit = 0; bit < 8; bit++)
    {
      uint8_t mask = 1 << bit;
      black = true;
      for (int col = 0; col < OLED::WIDTH; col++)
      {
        if (!black)
          pageBuffer[col] |= mask;
        ctr--;
        if (ctr == 0)
        {
          black = !black;
          ctr = pgm_read_byte(ptr++);
        }
      }
    }
    // send the page, mirroring the column bytes as we go
    OLED::StartPage(page);
    for (int col = 0; col < OLED::WIDTH; col++)
    {
      OLED::PageColumn(pageBuffer[col]);
      uint8_t mirror = 0;
      for (int bit = 0; bit < 8; bit++)
        if (pageBuffer[col] & (1 << bit))
          mirror |= 0x80 >> bit;
      pageBuffer[col] = mirror;
    }
    OLED::EndPage();
    // send the mirror page
    OLED::StartPage((OLED::HEIGHT/8) - page - 1);
    for (int col = 0; col < OLED::WIDTH; col++)
      OLED::PageColumn(pageBuffer[col]);
    OLED::EndPage();
  }
  delay(kTextLinger);
}

void Splash()
{
  // Splash image and text, with button help
  SplashImage();
  for (int i = 0; i < 4; i++)
  {
    OLEDText::DisplayLine(MSTR_StrN(pSplashLines0, i), 0, true, -1, (i < 2)?-1:0); // centre first to pages
    OLEDText::DisplayLine(MSTR_StrN(pSplashLines1, i), 1, true, -1, (i < 2)?-1:0);
    delay(kTextLinger);
  }
}

void ITOA(uint8_t i, char pad, char* pBuff)
{
  // appends i as two-digit number, padded with pad
  char num[3];
  num[0] = (i < 10)?pad:('0' + i / 10);
  num[1] = '0' + i % 10;
  num[2] = 0;
  strcat(pBuff, num);
}

#ifdef CFG_MOON_IS_APPROX
enum tPages {eDateTimePage, ePhasePage, eAdjustPage, eMonitorPage, eLDRPage, eTitlePage, eCreditPage, eLastPage};
#else
enum tPages {eDateTimePage, eMoonPage, ePhasePage, eAdjustPage, eMonitorPage, eLDRPage, eTitlePage, eCreditPage, eLastPage};
#endif
const uint8_t kBarWidth = 2;
// live updates are decimated, let's time colon blink, makes buttons more responsive
uint16_t pageDisplayCtr = 0;
bool blinkPage = false;

void DisplayPageScrollBar(uint8_t page)
{
  // draw a "scroll bar" at the far right indicating page position in list
  uint32_t bar0 = 0x55555555;
  uint32_t bar1 = 0xAAAAAAAA;
  uint32_t thumb = (1UL << (OLED::HEIGHT/eLastPage)) - 1;
  thumb <<= page*OLED::HEIGHT/eLastPage;
  bar0 |= thumb;
  bar1 |= thumb;
  for (uint8_t bank = 0; bank < OLED::HEIGHT/8; bank++)
  {
    OLED::StartPage(bank, OLED::WIDTH - 3);
    OLED::PageColumn(bar0 & 0xFF);
    OLED::PageColumn(bar1 & 0xFF);
    OLED::EndPage();  
    bar0 >>= 8;
    bar1 >>= 8;
  }
}

void DisplayDateTimePage()
{
  // Show date/time
  //   0123456789
  //   Mmm dd 'YY
  //   Dd hh:mm X   X is D/S for DLS/Std
  // Colon blinks
  char buff[16];
  ATC::Loop();
  ::strcpy_P(buff, MSTR_StrN(pMonthLongNames, ATC::_Month - 1));
  buff[3] = 0; // truncate
  ::strcat_P(buff, pDateExtras);
  ITOA(ATC::_Date, ' ', buff);
  ::strcat_P(buff, MSTR_StrN(pDateExtras, 1));
  ITOA(ATC::_Year, '0', buff);
  OLEDText::DisplayLine(buff, 0, false, OLED::WIDTH - kBarWidth);
  
  ::strcpy_P(buff, MSTR_StrN(pDayShortNames, ATC::_DayOfWeek));
  buff[2] = 0; // truncate
  ::strcat_P(buff, pDateExtras);
  if (ATC::_TimeSet)
  {
    ITOA(ATC::GetHour24(ATC::_Hour24), '0', buff);
    ::strcat_P(buff, MSTR_StrN(pDateExtras, blinkPage?0:2));
    ITOA(ATC::_Minute, '0', buff);
  }
  else
    ::strcat_P(buff, MSTR_StrN(pDateExtras, 5)); // ??:??
#ifdef CFG_TIME_USE_DLS  
  ::strcat_P(buff, pDateExtras);
  ::strcat_P(buff, MSTR_StrN(pDateExtras, ATC::_DLS?4:3));
#endif  
  OLEDText::DisplayLine(buff, 1, false, OLED::WIDTH - kBarWidth);
  pageDisplayCtr = 25; // regular updates, rapid blink to indicate it's live
  blinkPage = !blinkPage;
}

void DisplayMoonPage()
{
  // Show new moon
  //   0123456789
  //   New@ hh:mm
  //   In dd days
  // or
  //   New@
  //   No config
  if (_NewMoonReferenceSeconds && ATC::_TimeSet)
  {
    char buff[16];
    int d, h, m;
    Moon::CalcNextNewMoon(ATC::_Year, ATC::_Month, ATC::_Date, d, h, m);
    ::strcpy_P(buff, pMoonLines);
    ITOA(ATC::GetHour24(h), '0', buff);
    ::strcat_P(buff, MSTR_StrN(pDateExtras, 2));
    ITOA(m, '0', buff);
    OLEDText::DisplayLine(buff, 0, false);

    ::strcpy_P(buff, MSTR_StrN(pMoonLines, 1));
    ITOA(d, ' ', buff);
    ::strcat_P(buff, pDaysSuffix);
    OLEDText::DisplayLine(buff, 1, false);
  }
  else
  {
    OLEDText::DisplayLine(pMoonLines, 0, true);
    OLEDText::DisplayLine(pNoConfigStr, 1, true);
  }
}

void DisplayPhasePage()
{
  // Phase of moon
  //   0123456789
  //   Waxing
  //   Gibbous
  // or 
  //   Phase
  //   No config
  const char* pLine0 = MSTR_StrN(pMoonLines, 2);
  const char* pLine1 = pNoConfigStr;
  if (_NewMoonReferenceSeconds && ATC::_TimeSet)
  {
    int age = 0;
    Moon::GetPhaseName(Moon::CalcPhase(ATC::_Year, ATC::_Month, ATC::_Date, age), pLine0, pLine1);
  }
  OLEDText::DisplayLine(pLine0, 0, true);
  OLEDText::DisplayLine(pLine1, 1, true);
}

void DisplayAdjustPage()
{
  // Last adjustment
  //   0123456789
  //   Adj@ hh:mm   sunrise
  //   +nnm HH:MM   nn=delta, sunset
  // or 
  //   Adj@
  //   None/Off/Skip
  if (!_MonitorOn)
  {
    OLEDText::DisplayLine(pAdjustStrings, 0, true);  // Adj@
    OLEDText::DisplayLine(pOffOnItems, 1, true); // Off
  }
  else if (Monitor::SkippingNextAdjustment())
  {
    OLEDText::DisplayLine(pAdjustStrings, 0, true);  // Adj@
    OLEDText::DisplayLine(MSTR_StrN(pAdjustStrings, 5), 1, true); // Skip
  }
  else if (Monitor::_Adjusted)
  {
    int hh, mm;
    char buff[16];
    ::strcpy_P(buff, pAdjustStrings); // Adj@
    // sunrise
    Calendar::MinutesToTime(Monitor::_DawnAtMinutes, hh, mm);
    ITOA(ATC::GetHour24(hh), '0', buff);
    ::strcat_P(buff, MSTR_StrN(pDateExtras, 2));  // :
    ITOA(mm, '0', buff);
    OLEDText::DisplayLine(buff, 0, false);
    // delta    
    ::strcpy_P(buff, MSTR_StrN(pAdjustStrings, Monitor::_AdjustmentMinutes >= 0?1:2));  // +/-
    int adj = abs(Monitor::_AdjustmentMinutes);
    ::itoa(min(adj, 99), buff + ::strlen(buff), 10);
    ::strcat_P(buff, MSTR_StrN(pAdjustStrings, 3)); // m
    // sunset
    ::strcat_P(buff, pDateExtras);  // <space>
    if (adj < 10)
      ::strcat_P(buff, pDateExtras);  // <space>
    Calendar::MinutesToTime(Monitor::_DawnAtMinutes - Monitor::_NightTotalMinutes, hh, mm);
    ITOA(ATC::GetHour24(hh), '0', buff);
    ::strcat_P(buff, MSTR_StrN(pDateExtras, 2));  // :
    ITOA(mm, '0', buff);
    OLEDText::DisplayLine(buff, 1, false);
  }
  else
  {
    OLEDText::DisplayLine(pAdjustStrings, 0, true);  // Adj@
    OLEDText::DisplayLine(MSTR_StrN(pAdjustStrings, 4), 1, true); // None
  }
}

void DisplayMonitorPage()
{
  // Monitor chart
  //   0123456789
  //   Monitor
  //   GGGGGGGGGG   graphics 
  // Light/Dark in current interval, and averages of last 24h of intervals.
  // '|' is light, '.' is dark, checkerboard is N/A
  // Left is most recent.
  char buff[16];
  int ReadingsIntoInterval, IntervalsPerHour, TotalIntervals, TotalReadingsPerInterval;
  Monitor::IntervalStats(ReadingsIntoInterval, IntervalsPerHour, TotalIntervals, TotalReadingsPerInterval);
  ::strcpy_P(buff, pMonitorStr);
  OLEDText::DisplayLine(buff, 0, false, OLED::WIDTH - kBarWidth); // leave a gap so the bar doesn't blink

  // build graphics
  uint8_t pageBuffer[OLED::WIDTH];
  for (int page = 0; page < 2; page++)
  {
    ::memset(pageBuffer, 0, sizeof(pageBuffer));
    // readings in current interval
    for (int reading = 0; reading < TotalReadingsPerInterval; reading++)
    {
      // MS bit is at the bottom
      uint16_t column = (reading % 2)?0b0001010101010000:0b0000101010101000;  // N/A
      if (reading < ReadingsIntoInterval)
        column = Monitor::CurrentIntervalWasLight(reading)?0b0111111111111110:0b0100000000000000; // light/dark
      pageBuffer[reading*2] = page?column >> 8:column;
    }
    // 2x 12h prev intervals
    int gap = 0;
    for (int interval = 0; interval < 12*IntervalsPerHour; interval++)
    {
      int i = interval + page*12*IntervalsPerHour;
      uint8_t column = (interval % 2)?0b00010100:0b00001010;  // N/A
      if (i < TotalIntervals)
        column = Monitor::PrevIntervalWasLight(i)?0b00111111:0b00100000; // light/dark
      pageBuffer[2*TotalReadingsPerInterval + 4 + interval + 2*gap] = column;
      if (interval % IntervalsPerHour == (IntervalsPerHour-1))
        gap++;
    }

    // show graphic
    OLED::StartPage(2 + page);
    for (int col = 0; col < OLED::WIDTH - kBarWidth; col++)
      OLED::PageColumn(pageBuffer[col]);
    OLED::EndPage();
  }
}

void DisplayLDRPage()
{
  // LDR, live
  //   0123456789
  //   LDR (tt)   tt is the dark/light threshold
  //   nnnn=Day   nnnn is the raw LDR reading
  // or
  //   nnnn=Night
  char buff[16];
  ::strcpy_P(buff, pLDRStrings);
  ::itoa(CFG_MON_LDR_THRESHOLD, buff + ::strlen(buff), 10);
  ::strcat_P(buff, MSTR_StrN(pLDRStrings, 1));
  OLEDText::DisplayLine(buff, 0, false, OLED::WIDTH - kBarWidth); // leave a gap so the bar doesn't blink
  bool isDay;
  ::itoa(Monitor::RawLDR(isDay), buff, 10);
  ::strcat_P(buff, MSTR_StrN(pLDRStrings, isDay?2:3));
  OLEDText::DisplayLine(buff, 1, false, OLED::WIDTH - kBarWidth);
  pageDisplayCtr = 10;  // fast periodic updates
}

void DisplayPages()
{
  // Show the pages
  uint8_t page = 0;
  OLEDText::DisplayLine(pSetHeldLines, 0, true);
  OLEDText::DisplayLine(MSTR_StrN(pSetHeldLines, 1), 1, true);
  delay(1000);
  while (btns.Down() == ABTN::eSet)
    ;
  uint32_t timerMS = millis();
  bool drawPage = true;
  pageDisplayCtr = 0;
  blinkPage = false;
  while (true)
  {
    ABTN::tButton btn = btns.Pressed();
    unsigned long timeoutMS = kIdleIntervalMS;
    if (page == eLDRPage || page == eMonitorPage || page == eDateTimePage)
      timeoutMS *= 10UL;
    if (((millis() - timerMS) > timeoutMS) ||  btn == ABTN::eSet)
      break;
    else if (btn == ABTN::eSel)
    {
      timerMS = millis();
      page++;
      if (page >= eLastPage)
        page = 0;
      drawPage = true;
      pageDisplayCtr = 0;
      blinkPage = false;
    }
    if (drawPage)
    {
      drawPage = false;
      switch (page)
      {
        case eDateTimePage: // updates live, longer time-out
          if (!pageDisplayCtr)
            DisplayDateTimePage();
          drawPage = true;
          break;
#ifndef CFG_MOON_IS_APPROX
        case eMoonPage:
          DisplayMoonPage();
          break;
#endif          
        case ePhasePage:
          DisplayPhasePage();
          break;
        case eAdjustPage:
          DisplayAdjustPage();
          break;
        case eMonitorPage:
          DisplayMonitorPage();
          break;
        case eLDRPage:
          if (!pageDisplayCtr)
            DisplayLDRPage(); // updates live, longer time-out
          drawPage = true;
          break;
        case eTitlePage:
        case eCreditPage:
          OLEDText::DisplayLine(MSTR_StrN(pSplashLines0, page - eTitlePage), 0, true, -1, -1); // centre
          OLEDText::DisplayLine(MSTR_StrN(pSplashLines1, page - eTitlePage), 1, true, -1, -1);
          break;
      }
      DisplayPageScrollBar(page);
      if (pageDisplayCtr)
        pageDisplayCtr--;
    }
  }
}

void Init()
{
  // Intialise/load
  LoadBuildDate();
  Load();
}

void ScrubStep(bool white, bool red)
{
  // Show the countdown, do the colour fill, wait
  OLEDText::DisplayLine(LineBuffer, 1, false);
  eInkDisplay::Scrub(white, red);
  LineBuffer[0]--; // 9...0
  delay(5000);
}

void Configure()
{
  // Configure date time etc
  OLEDText::DisplayLine(NULL, 0, false);
  OLEDText::DisplayLine(NULL, 1, false);  
  OLED::On(true);
  uint8_t menu = 0;
  
  // Check for Set held
  uint32_t timerMS = millis();
  while (((millis() - timerMS) < kButtonHeldMS) && btns.Down() == ABTN::eSet)
    ;
  bool update = false;
  if (btns.Down() == ABTN::eSet)
  {
    // Set held -- view date/time/moon etc
    DisplayPages();
  }
  else while (true)
  {
    bool cont = false;
    if (Menu(pConfigure, menu, 0, eExit, pConfigureItems))
    {
      switch (menu)
      {
#ifdef CFG_TIME_USE_DLS
        case eDLS:
        {
          uint8_t value = ATC::_DLS;
          update = cont = Menu(MSTR_StrN(pConfigureItems, menu), value, 0, 1, pOffOnItems);
          if (cont)
          {
            ATC::_DLS = value;
            Save();
          }
          break;
        }
#endif        
        case eTime:
          update = cont = ConfigureTime();
          break;
        case eDate:
          update = cont = ConfigureDate();
          break;
        case eMoon:
          update = cont = ConfigureMoon();
          break;
        case eAdjust:
        {
          uint8_t value = _MonitorOn?1:0;
          cont = Menu(MSTR_StrN(pConfigureItems, menu), value, 0, 1, pOffOnItems);
          if (cont)
          {
            _MonitorOn = value;
            Save();
          }
          break;
        }
        case eScrub:
        {
          // Cycle all pixels black/white/red 3x
          OLEDText::DisplayLine(Config::pBusyStr, 0, true);
          OLEDText::DisplayLine(Config::pEmptyStr, 1, true);
          bool redWhite = true;
          LineBuffer[0] = '9';
          LineBuffer[1] = '\0';
          for (int pass = 0; pass < 3; pass++)
          {
            ScrubStep(false, false);    // black
            ScrubStep(true,  false);    // white
            ScrubStep(redWhite, true);  // (white/black) and red 
            redWhite = !redWhite;
          }
          ScrubStep(false, false);      // black
          update = true; // and exit
          break;
        }
        default:
          break;
      }
      menu++;
    }
    if (!cont)
      break;
  }
  OLEDText::DisplayLine(NULL, 0, false);
  OLEDText::DisplayLine(NULL, 1, false);
  OLED::On(false);
  if (update)
    DrawPage();
}

void Face()
{
  // Change face Moon/Calendar, or if held, skip/resume next adjustment
  // Check for Sel held
  uint32_t timerMS = millis();
  while (((millis() - timerMS) < kButtonHeldMS) && btns.Down() == ABTN::eSel)
    ;
  if (btns.Down() == ABTN::eSel)
  {
    // Sel held -- toggle skip next adjustment
    OLED::On(true);
    int idx = Monitor::ToggleSkipNextAdjustment()?0:1;
    OLEDText::DisplayLine(MSTR_StrN(pSkipAdjStr, idx), 0, true);
    OLEDText::DisplayLine(MSTR_StrN(pSkipAdjStr, 2), 1, true);
    delay(kTextLinger);
    OLED::On(false);
  }
  else
  {
    _Face = (_Face + 1) % 3;
    Save();
    DrawPage();
  }
}

void DrawPage()
{
  static bool sleeping = false;
  // Despite what the epd1in54b example says, Reset() is not enough to wake, WaitUntilIdle() is an infinite loop
  // Re-initialising does work
  if (sleeping)
    eInkDisplay::Init();
  if (_Face)
    Calendar::Draw(_Face == 1);
  else
    Moon::Draw();
  eInkDisplay::Sleep();
  sleeping = true;
}

}
