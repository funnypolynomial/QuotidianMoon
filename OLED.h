#pragma once

// 32x128 OLED display, used for configuration

// If defined, sends OLED data to Serial
//#define OLED_DUMP_IMAGE

namespace OLED
{
  const int HEIGHT = 32;
  const int WIDTH  = 128;
  void Init(int resetPin = -1); // means N/A
  void On(bool on);
  
  void StartPage(uint8_t page, uint8_t column = 0);
  void PageColumn(uint8_t data);
  void EndPage();
};
