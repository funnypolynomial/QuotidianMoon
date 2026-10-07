#pragma once

// eInk Display

// If defined, sends image data to Serial
//#define EINK_DUMP_IMAGE

namespace eInkDisplay
{
  const int WIDTH  = 200;
  const int HEIGHT = 200;
  enum Colour {MonoBlack, MonoGrey, MonoWhite, // 2 bpp
               ColourNone, ColourRed};         // 1 bpp, off = red
               // *NOTE*: I've seen ghosting with Red so I avoid it here (but see eScrub)
  
  void Init();
  void Reset();
  void Sleep();
  void SendCommand(uint8_t data);
  void SendData(uint8_t data);
  void StartMono();
  void StartRed();
  void Refresh();
  void Scrub(bool white, bool red);
  
  extern uint8_t rowBuffer[eInkDisplay::WIDTH/4];  // a single buffer with space for a row of mono or red pixels. 2 bits/1 bit per pixel
  void FillRowBuffer(Colour clr);
  void SetRowBufferAt(int col, Colour clr);
  void SetRowBufferAt(int col, Colour clr, int len);
  Colour GetRowBufferAt(int col);

  // writing pixels at a cursor pos, auto-advances
  void StartRowBufferWrite(int col = 0);
  int GetRowBufferWriteCol();
  void WriteRowBuffer(Colour clr, int len = 1);
  
  void SendRowBuffer();
  
  void FillRowBuffer(uint8_t* buff, Colour clr);
  void SetRowBufferAt(uint8_t* buff, int col, Colour clr);
  void SetRowBufferAt(uint8_t* buff, int col, Colour clr, int len);
  
  void SendRowBuffer(uint8_t* buff);
};
