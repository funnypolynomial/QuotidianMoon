#pragma once

// Configuration

// Serial on etc
//#define DEBUG

// Sent to serial
#ifdef DEBUG
#define DBG(_x) Serial.print(#_x);Serial.print(":");Serial.println(_x);
#else
#define DBG(_x)
#endif


// If defined, eInk display has connector at bottom
#define CFG_EINK_UPRIGHT_DISPLAY

// Use 8x8 font (vs 5x6) on the eInk display. Using only 5x6 saves ~870 program storage bytes.
// Only applies to the Moon and Date faces, Calendar always uses 5x6
#define CFG_EINK_8x8_FONT

// Controls orientation of moon, direction of phase
//#define CFG_MOON_NORTHERN_HEMISPHERE

// If defined, adds date etc around moon in the given colour (mono grey/white). No text if not defined.
#define CFG_MOON_TEXT_COLOUR        eInkDisplay::MonoGrey // or eInkDisplay::MonoWhite

// If defined, specifies the angular span (degrees) of the illuminated sliver, below which (thinner than)
// the moon will be drawn as a full faint grey disk.  Otherwise a sliver (or nothing). 
// See Moon::DrawGore() & bool greyDisk
#define CFG_MOON_GREY_DISK          15

// If defined, suppresses showing inaccurate numbers, the age, the next New Moon.
// Turns off overly optimistic code!
#define CFG_MOON_IS_APPROX

// Controls whether DLS is shown/used
#define CFG_TIME_USE_DLS

// An LDR reading above this is considered "bright" (day) ambient
#define CFG_MON_LDR_THRESHOLD       50

// Monitor adjustment rules
// Christchurch's shortest night is ~8.5h, longest ~15h. 
// See for example, https://www.timeanddate.com/sun/new-zealand/christchurch
#define CFG_MON_MIN_DARK_MINUTES    6*60    // Fewer than 6 hours of darkness and something's gone wrong? 
#define CFG_MON_MAX_DARK_MINUTES    18*60   // More than 18 hours of darkness and something's gone wrong?
#define CFG_MON_MAX_ADJUST_MINUTES  60      // More than an hour of adjustment is unlikely?

// 180E is NZST (Local Standard Time Meridian (LSTM)), Christchurch at 172.62E. 
// 4 minutes per degree.
// For every degree West of the LSTM, sunset/rise is 4 minutes later. +ve = LATER in zone
// Try to get a little closer to local time:
#define CFG_MON_MINUTES_INTO_ZONE   (4*(180 - 173))

// ATC defines
// Milliseconds lost (+ve) or gained (-ve) per minute. YMMV
#define CFG_ATC_LOSS_PER_MINUTE_MS      (-125UL)

// If defined, blinks the LED on the hour
//#define CFG_ATC_BLINK_LED_ON_HOUR

// Calendar defines, note there are other details set by const's in Calendar.cpp.
// Calendar lines, solid/dashed and colour (MonoBlack, MonoGrey or MonoWhite (=none))
//#define CFG_CAL_SOLID_LINES
#define CFG_CAL_LINES_COLOUR        eInkDisplay::MonoBlack
// Previous/Next month date text colour  (MonoGrey or MonoWhite (=none))
#define CFG_CAL_OTHER_MONTH_COLOUR  eInkDisplay::MonoGrey 
// First day of the week
#define CFG_CAL_FIRST_WEEKDAY       0 // 0=Sun, 1=Mon

namespace Config
{
  extern const char pMonthLongNames[]; // January, February etc
  extern const char pDayShortNames[];   // Sun, Mon etc
  extern const char pSplashLines0[];
  extern const char pSplashLines1[];
  extern const char pNoConfigStr[];
  extern const char pDateExtras[];
  extern const char pBusyStr[];
  extern const char pEmptyStr[];

  extern uint32_t  _NewMoonReferenceSeconds; // timestamp of reference New Moon
  extern bool      _MonitorOn;               // true if Monitor is adjusting time

  void Init();
  void Splash();
  void Configure();
  void Face();
  void Save();

  void DrawPage();
}

// Helpers for multi-string constants.
// Multi-strings are one or more strings concatenated into a single string, usually in PROGMEM.
// Strings are separated by a NUL.  The multi-string ends with two NUL's
#undef MSTR
#define MSTR(s) s "\0"  

// Extract the nth string from a multi-string
extern const char* MSTR_StrN(const char* multiStr, uint8_t n);
