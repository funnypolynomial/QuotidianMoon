#include <Arduino.h>
#include <SPI.h>
#include "ABTN.h"
#include "ATC.h"
#include "Config.h"
#include "eInkDisplay.h"
#include "Monitor.h"
#include "Moon.h"
#include "OLED.h"
#include "OLEDText.h"
#include "Pins.h"

//  Q u o t i d i a n M o o n
//  -------------------------
//   An experiment. Shows the Moon Phase (etc) without using an RTC (or NTP).
//
//   A small eInk display showing an image of the Moon's current phase (or a calendar page, or the date) and using an
//   LDR to regulate the approximate time and advance the date.  
//   A small OLED and two push-buttons for configuration.
//
//   **********************************************************
//   *  Needless to say, the Moon phase is quite approximate. *
//   *     *** For entertainment purposes only! ***           *
//   **********************************************************
//
// Major components:
// --- ATC (Approximate Time Clock) ---
//   Not a real time clock (RTC). The Approximate Time Clock is a software clock which updates the date and time
//   based on the oscillator built into the Arduino. This is inaccurate, drifting by seconds per hour.
//
// --- Monitor ---
//   The Monitor keeps the ATC drift in check, preventing any error from simply accumulating unbound.
//   Time is divided into 10 minute intervals. Every minute in an interval the LDR is read and classified as dark
//   or light. At the end of an interval, the average classification is computed and the whole interval is
//   classified as dark or light.
//   The Monitor looks for a single light interval preceded by a long enough unbroken series of dark intervals.
//   This is considered "dawn" and the assumption is that the block of dark intervals was night-time, and midnight
//   was at the midpoint of the block (solar midnight).  The ATC time is adjusted accordingly.
//   The ATC will always be a little off, but the error won't grow unchecked.
//   For our purposes, exact time isn't a requirement, just a consistent track of the date and a rough time of day.
//   The error will likely depend on a number of factors:
//     * the season (long or short days),
//     * the weather (very dark clouds),
//     * the location (in a room with East- or West-facing window),
//     * the occupants (early risers turning on lights in Winter)
//   There are some constraints on the Monitor's logic, the minimum and maximum "night" and the maximum shift in the
//   time. If these are not met, the adjustment is skipped.  See CFG_MON_*. Note also pressing-and-holding the SEL
//   button will force skipping the next adjustment.
//
//   CFG_MON_MINUTES_INTO_ZONE optionally specifies an offset from solar to local time. It, together with optional
//   CFG_TIME_USE_DLS & CFG_ATC_LOSS_PER_MINUTE_MS, attempt to make the ATC time shown closer to expectation.
//
// --- Moon ---
//   Computes the current phase based on a reference New Moon.  It draws the Moon's phase by rendering
//   "gores" from PROGMEM-encoded data built from a 100x100 pixel image (see MoonData.h and the resources subdirectory).
//   Each image pixel is drawn as a 2x2 pixel cell to support 16 levels.  The date etc is shown around the Moon
//   (optionally, see CFG_MOON_TEXT_COLOUR)
//   The Moon compression/rendering is taken from my https://github.com/funnypolynomial/ArDSKYlite project.
//
// --- Calendar ---
//   Draws a calendar grid with the current day highlighted. The style is configurable (see CFG_CAL_*)

// --- Date ---
//   Draws the Day, Date and Month
//
// --- eInk ---
//   The display is 200x200 with black, grey or white pixels; or red. It is used to show the Moon's phase or the
//   Calendar.
//
// --- OLED ---
//   A small 128x32 OLED display is used for configuration etc (the eInk display is just too slow to make live
//   interaction with it practical). Two lines of text, using the 5x6 font, double-sized.
//
// --- Config ---
//   A number of aspects of the project are compile-time configurable, see Config.h.  For example:
//     * If Daylight Savings Time is applied (CFG_TIME_USE_DLS)
//     * The orientation of the Moon (CFG_MOON_NORTHERN_HEMISPHERE)
//     * Day/Night LDR threshold (CFG_MON_LDR_THRESHOLD)
//
//   There is also a set of run-time configurable items, initiated by pressing the SET button. Within the config
//   mode, SET accepts the setting, SEL increments it.  The first screen selects the category to be configured:
//   SEL scrolls through them, SET starts editing them. The categories are:
//     * DLS : Toggles Daylight Savings Time (if applicable).
//     * Time: Sets the time; the hour (24-hour mode) and the minute (to the nearest 5mins).
//     * Date: Sets the date; the year, month and date. The day-of-week is computed.
//     * Moon: Sets the reference New Moon; the number of days until it is New and the time of day (nearest 5min).
//             If today is Wednesday and the New Moon is 8am on Friday, then it is at 08:00 in 2 days.
//     * Adjust: Toggles off and on the monitor adjusting the time.
//     * Scrub: Cycles the eInk display all black/white/red 3x
//     * Exit: Exits config mode.
//   There is a 30s idle time-out.
//
// --- Buttons ---
//   * Pressing SET configures time etc.
//   * Pressing SEL toggles the face between Moon, Calendar and Date.
//   * Holding SET (~3sec) shows a number of informational pages.  SEL steps through them, SET exits.  The pages are:
//       - Current date and time. If applicable, 'D' or 'S' follows the time, indicating DLS or Std time.
//       - Days to, and time of, the next New Moon. Omitted if CFG_MOON_IS_APPROX
//       - The current Moon phase, in words.
//       - The most recent Monitor adjustment (happened this morning). On the top line, the time sunrise/dawn
//         was detected. On the bottom line the adjustment in minutes, and the time sunset was detected.
//       - A graph of Monitor intervals showing light/dark, grouped into hours, also minutes into the current interval.
//       - The current live LDR reading and if it is classified as light or dark.  The threshold is shown in brackets.
//     There is a 30s idle time-out. "None"/"NO CONFIG" is shown if N/A.
//   * Holding SEL disables the next monitor adjustment.  Useful if ambient light usage means the adjustment will
//     likely be badly skewed.  Needs to be done before midnight of the night in question.
//     It's a toggle, holding again re-enables adjustment.
//   Note: the Monitor and ATC are inactive during configuration and while showing pages.
//
// --- Date & Time ---
//   The display and Monitor relies on an at-least approximate time and an accurate date. This is an issue if power is
//   lost.  To minimise this, every time it is configured, or rolls over at midnight, the current date is written to
//   EEPROM.
//   At power-up it is restored from there (or from the compile-time __DATE__ if missing). If nothing else it minimises the
//   work needed to set it correctly.
//   It will use the restored date without complaint, however, until the time is set, the Moon, Calendar and Date views
//   will indicate "NO CONFIG".
//   Additionally, the Moon requires that the reference New Moon be set.
//
// --- Pins.h ---
//   This includes pin assignments and an ASCII-Art schematic.
//
// --- Localization/Customisation ---
//   The sketch as it stands now is configured for my use in Christchurch, New Zealand:
//      * CFG_MOON_NORTHERN_HEMISPHERE is undefined so the Moon's appearance is for the Southern Hemisphere
//        (a First Quarter Moon has the dark area on the left)
//      * CFG_MON_MINUTES_INTO_ZONE is for Christchurch and NZ's Local Standard Time Meridian.
//      * CFG_ATC_LOSS_PER_MINUTE_MS is for my hardware setup. 
//        For example, running the ATC with the monitor off for 24 hours, I saw a gain of around 3 minutes 
//        or ~125ms per minute (early Winter, ~5 degree temperature variation, ~20C average).
//        Note the correction is probably moot, since the Monitor keeps the drift in check.
//   The physical location matters -- exposure to daylight and isolation from artificial light are important,
//   particularly during Winter.
//
// No AI.  It's about the journey as much as the destination.
//
// Mark Wilson, 2026

void setup() 
{
#ifdef DEBUG
  // Serial comms seem a little flakey on the Leonardo ETH
  Serial.begin(38400);
  // This helps?
  while (!Serial)
    ;
  Serial.println("; QuotidianMoon");
#endif
  btns.Init(PIN_BTNS, 900, 400);
  ATC::Init();
  Monitor::Init();
  OLED::Init();
  Config::Init();
#ifndef DEBUG  
  Config::Splash();
#endif  
  OLED::On(false);
  eInkDisplay::Init();
  Config::DrawPage();
}

void loop() 
{
  ATC::Loop();
  Monitor::Loop();
  ABTN::tButton btn = btns.Pressed();
  if (btn == ABTN::eSet)
    Config::Configure();
  else if (btn == ABTN::eSel)
    Config::Face();
  else if (Monitor::_NewDay)
  {
    // it's a new day, redraw the moon/calendar
    Monitor::_NewDay = false;
    Config::DrawPage();
  }
}
