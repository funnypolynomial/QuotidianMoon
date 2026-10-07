#include <Arduino.h>
#include <SPI.h>
#include "Config.h"
#include "eInkDisplay.h"
#include "Pins.h"
// Using Jaycar's 1.54 inch 200x200 e-Ink display https://www.jaycar.co.nz/duinotech-arduino-compatible-1-54-inch-monochrome-e-ink-display/p/XC3747
// https://www.waveshare.com/wiki/1.54inch_e-Paper_Module_Manual#Working_With_Arduino
// See 1.54inch e-Paper Datasheet（V1）
// Limited capability: no partial updates, no set orientation (display fills from the connector side)

namespace eInkDisplay
{
#define CMD_PANEL_SETTING                               0x00
#define CMD_POWER_SETTING                               0x01
#define CMD_POWER_OFF                                   0x02
#define CMD_POWER_ON                                    0x04
#define CMD_BOOSTER_SOFT_START                          0x06
#define CMD_DATA_START_TRANSMISSION_1                   0x10
#define CMD_DISPLAY_REFRESH                             0x12
#define CMD_DATA_START_TRANSMISSION_2                   0x13
#define CMD_PLL_CONTROL                                 0x30
#define CMD_VCOM_AND_DATA_INTERVAL_SETTING              0x50
#define CMD_TCON_RESOLUTION                             0x61
#define CMD_VCM_DC_SETTING_REGISTER                     0x82
  
void SetLUTs();
void SendCommand(uint8_t cmd);
void SendData(uint8_t data);
void WaitUntilIdle();
uint8_t rowBuffer[eInkDisplay::WIDTH/4];  // a single buffer with space for a row of mono or red pixels

bool monoBufferMode = true;  // set in StartMono()/StartColour(), not by colour setting

void Init()
{
  ::memset(rowBuffer, 0, sizeof(rowBuffer));
  pinMode(PIN_DISPLAY_CS,   OUTPUT);
  pinMode(PIN_DISPLAY_RST,  OUTPUT);
  pinMode(PIN_DISPLAY_DC,   OUTPUT);
  pinMode(PIN_DISPLAY_BUSY, INPUT); 
  SPI.begin();
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  
  // hardware init
  Reset();
  SendCommand(CMD_POWER_SETTING);
  SendData(0x07);
  SendData(0x00);
  SendData(0x08);
  SendData(0x00);
  SendCommand(CMD_BOOSTER_SOFT_START);
  SendData(0x07);
  SendData(0x07);
  SendData(0x07);
  SendCommand(CMD_POWER_ON);
  
  WaitUntilIdle();
  
  SendCommand(CMD_PANEL_SETTING); // 0x00 is not a real command?
  SendData(0xCF);
  SendCommand(CMD_VCOM_AND_DATA_INTERVAL_SETTING);
  SendData(0x17);
  SendCommand(CMD_PLL_CONTROL);
  SendData(0x39);
  SendCommand(CMD_TCON_RESOLUTION);
  SendData(0xC8);
  SendData(0x00);
  SendData(0xC8);
  SendCommand(CMD_VCM_DC_SETTING_REGISTER);
  SendData(0x0E);
  
  SetLUTs();
}

void Reset()
{
  digitalWrite(PIN_DISPLAY_RST, LOW); 
  delay(200);
  digitalWrite(PIN_DISPLAY_RST, HIGH);
  delay(200);      
}

void SendCommand(uint8_t cmd)
{
  digitalWrite(PIN_DISPLAY_DC, LOW);
  digitalWrite(PIN_DISPLAY_CS, LOW);
  SPI.transfer(cmd);
  digitalWrite(PIN_DISPLAY_CS, HIGH);
}

void SendData(uint8_t data)
{
  digitalWrite(PIN_DISPLAY_DC, HIGH);
  digitalWrite(PIN_DISPLAY_CS, LOW);
  SPI.transfer(data);
  digitalWrite(PIN_DISPLAY_CS, HIGH);
}

void StartMono()
{
  // start updating the mono buffer
  monoBufferMode = true;
  delay(2);
  SendCommand(CMD_DATA_START_TRANSMISSION_1);
  delay(2);
#ifdef EINK_DUMP_IMAGE
#ifdef CFG_EINK_UPRIGHT_DISPLAY
  Serial.println('I');
#endif  
  Serial.println('M');
#endif  
}

void StartRed()
{
  // start updating the red buffer
  monoBufferMode = false;
  delay(2);
  SendCommand(CMD_DATA_START_TRANSMISSION_2);
  delay(2);
#ifdef EINK_DUMP_IMAGE
  Serial.println('R');
#endif  
}

void WaitUntilIdle()
{
  // wait until busy goes high (or timeout!)
  int ctr = 100;
  while (!digitalRead(PIN_DISPLAY_BUSY) && --ctr)
    delay(100);
}

void Refresh()
{
  // update the display from the buffers
  delay(2);
  SendCommand(CMD_DISPLAY_REFRESH);
  WaitUntilIdle();
}

void Scrub(bool white, bool red)
{
  // Fill the screen with colour
  Init();
  StartMono();
  FillRowBuffer(white?MonoWhite:MonoBlack);
  for (int row = 0; row < HEIGHT; row++)
    SendRowBuffer(rowBuffer);

  StartRed();
  FillRowBuffer(red?ColourRed:ColourNone);
  for (int row = 0; row < HEIGHT; row++)
    SendRowBuffer(rowBuffer);

  Refresh();
  Sleep();
}


void Sleep()
{
  // enter deep sleep, call Reset to rewake (NOT TRUE, cal Init())
  SendCommand(CMD_VCOM_AND_DATA_INTERVAL_SETTING);
  SendData(0x17);
  SendCommand(CMD_VCM_DC_SETTING_REGISTER);  // to solve Vcom drop
  SendData(0x00);
  SendCommand(CMD_POWER_SETTING);  // power setting
  SendData(0x02);  // gate switch to external
  SendData(0x00);
  SendData(0x00);
  SendData(0x00);
  WaitUntilIdle();
  SendCommand(CMD_POWER_OFF);  // power off
}

static const uint8_t pLUTData[] PROGMEM =
{
  // Monochrome
  0x20,  // lut_vcom0
    0x0E, 0x14, 0x01, 0x0A, 0x06, 0x04, 0x0A, 0x0A,
    0x0F, 0x03, 0x03, 0x0C, 0x06, 0x0A, 0x00,
  0x21,  // lut_w
    0x0E, 0x14, 0x01, 0x0A, 0x46, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x86, 0x0A, 0x04,
  0x22,  // lut_b
    0x0E, 0x14, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x06, 0x4A, 0x04,
  0x23,  // lut_g1
    0x8E, 0x94, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x06, 0x0A, 0x04,
  0x24,  // lut_g2
    0x8E, 0x94, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x06, 0x0A, 0x04,

  // Red
  0x25,  // lut_vcom1
    0x03, 0x1D, 0x01, 0x01, 0x08, 0x23, 0x37, 0x37,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x26,  // lut_red0
    0x83, 0x5D, 0x01, 0x81, 0x48, 0x23, 0x77, 0x77,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x27,  // lut_red1
    0x03, 0x1D, 0x01, 0x01, 0x08, 0x23, 0x37, 0x37,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    
  0x00
};

void SetLUTs()
{
  // set look-up tables
  const uint8_t* pLUT = pLUTData;
  while (pgm_read_byte_near(pLUT))
  {
    SendCommand(pgm_read_byte_near(pLUT++));
    for (int i = 0; i < 15; i++)
      SendData(pgm_read_byte_near(pLUT++));
  }
}

void FillRowBuffer(uint8_t* buff, Colour clr)
{
  // set all pixels in row buffer
  uint8_t val = 0b00000000;
  switch (clr)
  {
    case MonoGrey   : val = 0b10101010; break;
    case MonoWhite  : val = 0b11111111; break;
    case ColourNone : val = 0b11111111; break;
    default         : val = 0b00000000; break;
  }
  ::memset(buff, val, eInkDisplay::WIDTH/4);  
}    

void SetRowBufferAt(uint8_t* buff, int col, Colour clr)
{
  // set a pixel in the row buffer
  uint8_t val = 0b00000000;
  if (col >= 200)
    return;
  if (clr < ColourNone)
  {
    // 2 bpp
    if (clr != MonoBlack)
      val = (clr == MonoWhite)?0b11000000:0b10000000;
    uint8_t lsr = 2*(col % 4);
    col >>= 2;
    buff[col] &= ~(0b11000000 >> lsr);
    buff[col] |=  (val        >> lsr);
  }
  else
  {
    // 1 bpp
    if (clr != ColourRed)
      val = 0b10000000;
    uint8_t lsr = col % 8;
    col >>= 3;
    buff[col] &= ~(0b10000000 >> lsr);
    buff[col] |=  (val        >> lsr);
  }
}

void SetRowBufferAt(uint8_t* buff, int col, Colour clr, int len)
{
  // set a series of pixel in the row buffer
  while (len--)
    SetRowBufferAt(buff, col++, clr);
}

int rowBufferWriteCol = 0;
void StartRowBufferWrite(int col /*= 0*/)
{
  // start writing pixels in the row buffer
  rowBufferWriteCol = col;
}

int GetRowBufferWriteCol()
{
  // return the current write position
  return rowBufferWriteCol;
}

void WriteRowBuffer(Colour clr, int len /*= 1*/)
{
  // set a line of pixels in the row buffer, at the current position (see StartRowBuffer())
  // updates position
  SetRowBufferAt(rowBufferWriteCol, clr, len);
  rowBufferWriteCol += len;
}

void SendRowBuffer(uint8_t* buff)
{
  // send the entire row of pixels to the display
  uint8_t* pBuffer = buff;
  if (monoBufferMode)
    for (size_t i = 0; i < eInkDisplay::WIDTH/4; i++)
      SendData(*pBuffer++);
  else
    for (size_t i = 0; i < eInkDisplay::WIDTH/8; i++)
      SendData(*pBuffer++);

#ifdef EINK_DUMP_IMAGE
  // Dump output is just the hex row data, 1 line per row
  pBuffer = buff;
  if (monoBufferMode)
  {
    // 4 pixels per byte, leftmost pixel is upper bits
    for (size_t i = 0; i < eInkDisplay::WIDTH/4; i++)
    {
      if (*pBuffer < 0x10)
        Serial.print('0');
      Serial.print(*pBuffer++, HEX);
    }
    Serial.println();
  }
  else
  {
    // 8 pixels per byte, leftmost pixel is upper bit
    for (size_t i = 0; i < eInkDisplay::WIDTH/8; i++)
    {
      if (*pBuffer < 0x10)
        Serial.print('0');
      Serial.print(*pBuffer++, HEX);
    }
    Serial.println();
  }
#endif
}

void FillRowBuffer(Colour clr)
{
  FillRowBuffer(rowBuffer, clr);
}

void SetRowBufferAt(int col, Colour clr)
{
  SetRowBufferAt(rowBuffer, col, clr);
}

void SetRowBufferAt(int col, Colour clr, int len)
{
  SetRowBufferAt(rowBuffer, col, clr, len);
}

Colour GetRowBufferAt(int col)
{
  // get a pixel from the row buffer
  if (monoBufferMode)
  {
    // 2 bpp
    uint8_t lsl = 2*(col % 4);
    col >>= 2;
    uint8_t val = (rowBuffer[col] << lsl) & 0b11000000;
    if (val == 0b11000000)
      return MonoWhite;
    else if (val == 0b10000000)
      return MonoGrey;
    else
      return MonoBlack;
  }
  else
  {
    // 1 bpp
    uint8_t lsl = col % 8;
    col >>= 3;
    uint8_t val = (rowBuffer[col] << lsl) & 0b10000000;
    if (val)
      return ColourNone;
    else
      return ColourRed;
  }
}

void SendRowBuffer()
{
  // send the entire row of pixels to the display
  SendRowBuffer(rowBuffer);
}
}
