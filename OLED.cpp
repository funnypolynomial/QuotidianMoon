#include <Arduino.h>
#include <Wire.h>
#include "OLED.h"
#include "OLEDText.h"
#include "Pins.h"

namespace OLED
{
// OLED_DUMP_IMAGE
// Format is 
//  P <page> <start-col n> <col-data n> <col-data n+1> <col-data n+2>...
  
// Gory details gleaned (and simplified) from 
// https://github.com/adafruit/Adafruit_SSD1306 and 
// https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
#define I2C_ADDR 0x3C
#define I2C_BUFFER_LENGTH 32

// Control byte D/C bit:
#define CMD_CONTROL_COMMAND     0x00
#define CMD_CONTROL_DATA        0x40

// commands
#define CMD_MEMORY_MODE         0x20
#define CMD_COLUMN_ADDR         0x21
#define CMD_PAGE_ADDR           0x22
#define CMD_SET_CONTRAST        0x81
#define CMD_CHARGE_PUMP         0x8D
#define CMD_SEG_REMAP           0xA0
#define CMD_ALL_ON_RESUME       0xA4
#define CMD_NORMAL_DISPLAY      0xA6
#define CMD_SET_MULTIPLEX       0xA8
#define CMD_DISPLAY_OFF         0xAE
#define CMD_DISPLAY_ON          0xAF
#define CMD_COM_SCAN_DEC        0xC8
#define CMD_SET_OFFSET          0xD3
#define CMD_SET_CLOCK_DIV       0xD5
#define CMD_SET_PRECHARGE       0xD9
#define CMD_SET_COM_PINS        0xDA
#define CMD_SET_VCOM_DETECT     0xDB
#define CMD_SET_START_LINE      0x40
#define CMD_DEACTIVATE_SCROLL   0x2E

void Command(uint8_t c)
{
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(CMD_CONTROL_COMMAND);
  Wire.write(c);
  Wire.endTransmission();
}

void CommandList(const uint8_t *c, uint8_t n)
{
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(CMD_CONTROL_COMMAND);
  uint16_t bytesOut = 1;
  while (n--)
  {
    if (bytesOut >= I2C_BUFFER_LENGTH)
    {
      Wire.endTransmission();
      Wire.beginTransmission(I2C_ADDR);
      Wire.write(CMD_CONTROL_COMMAND);
      bytesOut = 1;
    }
    Wire.write(pgm_read_byte(c++));
    bytesOut++;
  }
  Wire.endTransmission();
}

// The 32x128 pixel display is 4 pages of 128 bytes
// Each byte is a column of pixels, lsb is top
uint16_t pageBytes = 0; // track bytes sent
void StartPage(uint8_t page, uint8_t column)
{
  // start writing bytes to the page (and optional column)
  Command(CMD_PAGE_ADDR);
  Command(page);
  Command(0xFF);
  
  Command(CMD_COLUMN_ADDR);
  Command(column);
  Command((WIDTH - 1));  
  pageBytes = 1;
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(CMD_CONTROL_DATA);
#ifdef OLED_DUMP_IMAGE  
  Serial.print("P ");Serial.print(page);Serial.print(" ");Serial.print(column);
#endif
}

void PageColumn(uint8_t data)
{
  // write a byte to the page (handle buffer o/f)
  if (pageBytes >= I2C_BUFFER_LENGTH)
  {
    Wire.endTransmission();
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(CMD_CONTROL_DATA);
    pageBytes = 1;
  }
  Wire.write(data);
  pageBytes++;
#ifdef OLED_DUMP_IMAGE  
  Serial.print(" ");Serial.print(data);
#endif
}

void EndPage()
{
  // end writing bytes to the page
  Wire.endTransmission();
#ifdef OLED_DUMP_IMAGE  
  Serial.println();
#endif
}

void On(bool on)
{
  // display on/off
  Command(on?CMD_DISPLAY_ON:CMD_DISPLAY_OFF);
}

static const uint8_t PROGMEM initCommands[] = 
{
  CMD_DISPLAY_OFF,
  CMD_SET_CLOCK_DIV,
  0x80,
  CMD_SET_MULTIPLEX,
  HEIGHT - 1,
  
  CMD_SET_OFFSET,
  0x00,
  CMD_SET_START_LINE | 0x00,
  CMD_CHARGE_PUMP, 0x14,

  CMD_MEMORY_MODE,  0x00,
  CMD_SEG_REMAP | 0x1,
  CMD_COM_SCAN_DEC,
  
  CMD_SET_COM_PINS, 0x02,
  CMD_SET_CONTRAST, 0x8F,

  CMD_SET_PRECHARGE, 0xF1,
  
  CMD_SET_VCOM_DETECT, 0x40,
  CMD_ALL_ON_RESUME,
  CMD_NORMAL_DISPLAY,
  CMD_DEACTIVATE_SCROLL,
  CMD_DISPLAY_ON
};  
  
void Init(int resetPin)
{
  Wire.begin();
  
  // Seems reset is needed, power-on isn't sufficient (for Adafruit's 6-pin version)
  if (resetPin != -1)
  {
    pinMode(resetPin, OUTPUT);
    digitalWrite(resetPin, HIGH);
    delay(1);
    digitalWrite(resetPin, LOW); 
    delay(10);
    digitalWrite(resetPin, HIGH);
  }    
  CommandList(initCommands, sizeof(initCommands));
  // blank it
  OLEDText::DisplayLine(NULL, 0, false, WIDTH + 1); // seems there may actually be an extra column?
  OLEDText::DisplayLine(NULL, 1, false, WIDTH + 1);  
}

}
