#include <Arduino.h>
#include "Font5x6.h"

#define DESCENDER 0x80000000UL
// Character 5 cols x 6 rows packed into 32 bits, bit 31 is descender flag, bit 30 could be <narrow> flag (but isn't)
#define X 1
#define _ 0
// font row
#define FR(a,b,c,d,e) (unsigned long)((e << 0) | (d << 1) | (c << 2) | (b << 3) | (a << 4)) // note the order leftmost pixel is msb
// font char
#define FC(a,b,c,d,e,f) (unsigned long)((a << 0) | (b << 5) | (c << 10) | (d << 15) | (e << 20) | (f << 25))
static const unsigned long pFont5x6[] PROGMEM =
{
    // Uppercase letters
    FC(FR(_,X,X,X,_),   // A
       FR(X,_,_,_,X),
       FR(X,X,X,X,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X)),
    FC(FR(X,X,X,X,_),   // B
       FR(X,_,_,_,X),
       FR(X,X,X,X,_),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,X,X,X,_)),
    FC(FR(_,X,X,X,_),   // C
       FR(X,_,_,_,X),
       FR(X,_,_,_,_),
       FR(X,_,_,_,_),
       FR(X,_,_,_,X),
       FR(_,X,X,X,_)),
    FC(FR(X,X,X,X,_),   // D
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,X,X,X,_)),
    FC(FR(X,X,X,X,X),   // E
       FR(X,_,_,_,_),
       FR(X,X,X,X,X),
       FR(X,_,_,_,_),
       FR(X,_,_,_,_),
       FR(X,X,X,X,X)),
    FC(FR(X,X,X,X,X),   // F
       FR(X,_,_,_,_),
       FR(X,X,X,X,X),
       FR(X,_,_,_,_),
       FR(X,_,_,_,_),
       FR(X,_,_,_,_)),
    FC(FR(_,X,X,X,X),   // G
       FR(X,_,_,_,_),
       FR(X,_,X,X,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(_,X,X,X,_)),
    FC(FR(X,_,_,_,X),   // H
       FR(X,_,_,_,X),
       FR(X,X,X,X,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X)),
    FC(FR(X,X,X,X,X),   // I
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(X,X,X,X,X)),
    FC(FR(X,X,X,X,X),   // J
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(X,X,_,_,_)),
    FC(FR(X,_,_,X,_),   // K
       FR(X,_,X,_,_),
       FR(X,X,_,_,_),
       FR(X,_,X,_,_),
       FR(X,_,_,X,_),
       FR(X,_,_,_,X)),
    FC(FR(X,_,_,_,_),   // L
       FR(X,_,_,_,_),
       FR(X,_,_,_,_),
       FR(X,_,_,_,_),
       FR(X,_,_,_,_),
       FR(X,X,X,X,X)),
    FC(FR(X,X,X,X,X),   // M
       FR(X,_,X,_,X),
       FR(X,_,X,_,X),
       FR(X,_,X,_,X),
       FR(X,_,X,_,X),
       FR(X,_,X,_,X)),
    FC(FR(X,_,_,_,X),   // N
       FR(X,_,_,_,X),
       FR(X,X,_,_,X),
       FR(X,_,X,_,X),
       FR(X,_,_,X,X),
       FR(X,_,_,_,X)),
    FC(FR(_,X,X,X,_),   // O
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(_,X,X,X,_)),
    FC(FR(X,X,X,X,_),   // P
       FR(X,_,_,_,X),
       FR(X,X,X,X,_),
       FR(X,_,_,_,_),
       FR(X,_,_,_,_),
       FR(X,_,_,_,_)),
    FC(FR(_,X,X,X,_),   // Q
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,X,_,X),
       FR(X,_,_,X,X),
       FR(_,X,X,X,X)),
    FC(FR(X,X,X,X,_),   // R
       FR(X,_,_,_,X),
       FR(X,X,X,X,_),
       FR(X,_,X,_,_),
       FR(X,_,_,X,_),
       FR(X,_,_,_,X)),
    FC(FR(_,X,X,X,X),   // S
       FR(X,_,_,_,_),
       FR(_,X,X,X,_),
       FR(_,_,_,_,X),
       FR(X,_,_,_,X),
       FR(_,X,X,X,_)),
    FC(FR(X,X,X,X,X),   // T
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_)),
    FC(FR(X,_,_,_,X),   // U
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(_,X,X,X,_)),
    FC(FR(X,_,_,_,X),   // V
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(_,X,_,X,_),
       FR(_,_,X,_,_)),
    FC(FR(X,_,X,_,X),   // W
       FR(X,_,X,_,X),
       FR(X,_,X,_,X),
       FR(X,_,X,_,X),
       FR(X,_,X,_,X),
       FR(X,X,X,X,X)),
    FC(FR(X,_,_,_,X),   // X
       FR(_,X,_,X,_),
       FR(_,_,X,_,_),
       FR(_,X,_,X,_),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X)),
    FC(FR(X,_,_,_,X),   // Y
       FR(X,_,_,_,X),
       FR(_,X,_,X,_),
       FR(_,_,X,_,_),
       FR(_,X,_,_,_),
       FR(X,_,_,_,_)),
    FC(FR(X,X,X,X,X),   // Z
       FR(_,_,_,X,_),
       FR(_,_,X,_,_),
       FR(_,X,_,_,_),
       FR(X,_,_,_,_),
       FR(X,X,X,X,X)),
/////////////////////
#ifdef FONT5x6_LOWERCASE
    // Lowercase letters
   //  FC(FR(_,_,_,_,_),   // a
   //     FR(_,X,X,X,X),
   //     FR(X,_,_,_,X),
   //     FR(X,_,_,_,X),
   //     FR(X,_,_,X,X),
   //     FR(_,X,X,_,X)),
    FC(FR(_,_,_,_,_),   // a
       FR(X,X,X,X,_),
       FR(_,_,_,_,X),
       FR(X,X,X,X,X),
       FR(X,_,_,_,X),
       FR(_,X,X,X,X)),       
    FC(FR(X,_,_,_,_),   // b
       FR(X,X,X,X,_),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,X,X,X,_)),
    FC(FR(_,_,_,_,_),   // c
       FR(_,X,X,X,X),
       FR(X,_,_,_,_),
       FR(X,_,_,_,_),
       FR(X,_,_,_,_),
       FR(_,X,X,X,X)),
    FC(FR(_,_,_,_,X),   // d
       FR(_,X,X,_,X),
       FR(X,_,_,X,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(_,X,X,X,X)),
    FC(FR(_,_,_,_,_),   // e
       FR(_,X,X,X,_),
       FR(X,_,_,_,X),
       FR(X,X,X,X,_),
       FR(X,_,_,_,_),
       FR(_,X,X,X,X)),
    FC(FR(_,_,X,X,X),   // f
       FR(_,X,_,_,_),
       FR(X,X,X,X,_),
       FR(_,X,_,_,_),
       FR(_,X,_,_,_),
       FR(_,X,_,_,_)),
#ifdef FONT5x6_DESCENDERS
    FC(FR(_,X,X,X,X),   // g
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,X,X,X,X),
       FR(_,_,_,_,X),
       FR(X,X,X,X,_)) | DESCENDER,
#else
    FC(FR(_,_,_,_,_),
       FR(_,X,X,X,X),
       FR(X,_,_,_,X),
       FR(X,X,X,X,X),
       FR(_,_,_,_,X),
       FR(X,X,X,X,_)),
#endif       
    FC(FR(X,_,_,_,_),   // h
       FR(X,_,X,X,_),
       FR(X,X,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X)),
    FC(FR(_,_,X,_,_),   // i
       FR(_,_,_,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_)),
#ifdef FONT5x6_DESCENDERS
    FC(FR(_,_,X,_,_),   // j
       FR(_,_,_,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(X,X,_,_,_)) | DESCENDER,
#else
    FC(FR(_,_,X,_,_),
       FR(_,_,_,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(X,X,_,_,_)),
#endif       
    FC(FR(X,_,_,_,_),   // k
       FR(X,_,_,_,X),
       FR(X,_,_,X,_),
       FR(X,X,X,_,_),
       FR(X,_,_,X,_),
       FR(X,_,_,_,X)),
    FC(FR(_,_,X,_,_),   // l
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_)),
    FC(FR(_,_,_,_,_),   // m
       FR(X,X,_,X,_),
       FR(X,_,X,_,X),
       FR(X,_,X,_,X),
       FR(X,_,X,_,X),
       FR(X,_,X,_,X)),
    FC(FR(_,_,_,_,_),   // n
       FR(X,X,X,X,_),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X)),
    FC(FR(_,_,_,_,_),   // o
       FR(_,X,X,X,_),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(_,X,X,X,_)),
#ifdef FONT5x6_DESCENDERS
    FC(FR(X,X,X,X,_),   // p
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,X,X,X,_),
       FR(X,_,_,_,_),
       FR(X,_,_,_,_)) | DESCENDER,
    FC(FR(_,X,X,X,X),   // q
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(_,X,X,X,X),
       FR(_,_,_,X,_),
       FR(_,_,_,X,X)) | DESCENDER,
#else       
    FC(FR(_,_,_,_,_),
       FR(X,X,X,X,_),
       FR(X,_,_,_,X),
       FR(X,X,X,X,_),
       FR(X,_,_,_,_),
       FR(X,_,_,_,_)),
    FC(FR(_,_,_,_,_),
       FR(_,X,X,X,_),
       FR(X,_,_,X,_),
       FR(_,X,X,X,_),
       FR(_,_,_,X,_),
       FR(_,_,_,X,X)),
#endif       
    FC(FR(_,_,_,_,_),   // r
       FR(X,_,X,X,X),
       FR(X,X,_,_,_),
       FR(X,_,_,_,_),
       FR(X,_,_,_,_),
       FR(X,_,_,_,_)),
    FC(FR(_,_,_,_,_),   // s
       FR(_,X,X,X,X),
       FR(X,_,_,_,_),
       FR(_,X,X,X,_),
       FR(_,_,_,_,X),
       FR(X,X,X,X,_)),
    FC(FR(_,X,_,_,_),   // t
       FR(_,X,_,_,_),
       FR(X,X,X,X,_),
       FR(_,X,_,_,_),
       FR(_,X,_,_,_),
       FR(_,_,X,X,X)),
    FC(FR(_,_,_,_,_),   // u
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(_,X,X,X,X)),
    FC(FR(_,_,_,_,_),   // v
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(_,X,_,X,_),
       FR(_,_,X,_,_)),
    FC(FR(_,_,_,_,_),   // w
       FR(X,_,X,_,X),
       FR(X,_,X,_,X),
       FR(X,_,X,_,X),
       FR(X,_,X,_,X),
       FR(_,X,_,X,_)),
    FC(FR(_,_,_,_,_),   // x
       FR(X,_,_,_,X),
       FR(_,X,_,X,_),
       FR(_,_,X,_,_),
       FR(_,X,_,X,_),
       FR(X,_,_,_,X)),
#ifdef FONT5x6_DESCENDERS
    FC(FR(X,_,_,_,X),   // y
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(_,X,X,X,X),
       FR(_,_,_,_,X),
       FR(X,X,X,X,_)) | DESCENDER,
#else
    FC(FR(_,_,_,_,_),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(_,X,X,X,X),
       FR(_,_,_,_,X),
       FR(X,X,X,X,_)),
#endif       
    FC(FR(_,_,_,_,_),   // z
       FR(X,X,X,X,X),
       FR(_,_,_,X,_),
       FR(_,_,X,_,_),
       FR(_,X,_,_,_),
       FR(X,X,X,X,X)),
#endif
/////////////////////
    // Digits
    FC(FR(_,X,X,X,_),   // 0
       FR(X,_,_,_,X),
       FR(X,_,_,X,X),
       FR(X,_,X,_,X),
       FR(X,X,_,_,X),
       FR(_,X,X,X,_)),
    FC(FR(_,X,X,_,_),   // 1
       FR(X,_,X,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(X,X,X,X,X)),
    FC(FR(_,X,X,X,_),   // 2
       FR(X,_,_,_,X),
       FR(_,_,_,X,_),
       FR(_,_,X,_,_),
       FR(_,X,_,_,_),
       FR(X,X,X,X,X)),
    FC(FR(X,X,X,X,_),   // 3
       FR(_,_,_,_,X),
       FR(_,X,X,X,_),
       FR(_,_,_,_,X),
       FR(_,_,_,_,X),
       FR(X,X,X,X,_)),
    FC(FR(X,_,X,_,_),   // 4
       FR(X,_,X,_,_),
       FR(X,X,X,X,X),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_)),
    FC(FR(X,X,X,X,X),   // 5
       FR(X,_,_,_,_),
       FR(X,X,X,X,_),
       FR(_,_,_,_,X),
       FR(_,_,_,_,X),
       FR(X,X,X,X,_)),
    FC(FR(_,X,X,X,X),   // 6
       FR(X,_,_,_,_),
       FR(X,X,X,X,_),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(_,X,X,X,_)),
    FC(FR(X,X,X,X,X),   // 7
       FR(_,_,_,_,X),
       FR(_,_,_,X,_),
       FR(_,_,X,_,_),
       FR(_,X,_,_,_),
       FR(X,_,_,_,_)),
    FC(FR(_,X,X,X,_),   // 8
       FR(X,_,_,_,X),
       FR(_,X,X,X,_),
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(_,X,X,X,_)),
    FC(FR(_,X,X,X,_),   // 9
       FR(X,_,_,_,X),
       FR(X,_,_,_,X),
       FR(_,X,X,X,X),
       FR(_,_,_,_,X),
       FR(X,X,X,X,_)),
/////////////////////
    // Some extras, see pFontExtras
    FC(FR(_,_,_,X,_),   // (
       FR(_,_,X,_,_),
       FR(_,X,_,_,_),
       FR(_,X,_,_,_),
       FR(_,_,X,_,_),
       FR(_,_,_,X,_)),
    FC(FR(_,X,_,_,_),   // )
       FR(_,_,X,_,_),
       FR(_,_,_,X,_),
       FR(_,_,_,X,_),
       FR(_,_,X,_,_),
       FR(_,X,_,_,_)),
    FC(FR(_,_,_,_,_),   // +
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(X,X,X,X,X),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_)),
    FC(FR(_,_,_,_,_),   // -
       FR(_,_,_,_,_),
       FR(_,_,_,_,_),
       FR(X,X,X,X,X),
       FR(_,_,_,_,_),
       FR(_,_,_,_,_)),
    FC(FR(_,_,_,_,_),   // :
       FR(_,_,_,_,_),
       FR(_,_,X,_,_),
       FR(_,_,_,_,_),
       FR(_,_,_,_,_),
       FR(_,_,X,_,_)),
    FC(FR(_,X,X,X,_),   // @
       FR(X,_,X,_,X),
       FR(X,_,X,X,X),
       FR(X,_,_,_,_),
       FR(X,_,_,_,X),
       FR(_,X,X,X,_)),
    FC(FR(_,_,X,_,_),   // '
       FR(_,_,X,_,_),
       FR(_,_,_,_,_),
       FR(_,_,_,_,_),
       FR(_,_,_,_,_),
       FR(_,_,_,_,_)),
    FC(FR(_,_,_,_,_),   // =
       FR(X,X,X,X,X),
       FR(_,_,_,_,_),
       FR(_,_,_,_,_),
       FR(X,X,X,X,X),
       FR(_,_,_,_,_)),
    FC(FR(_,X,X,X,_),   // ?
       FR(X,_,_,_,X),
       FR(X,_,_,X,_),
       FR(_,_,X,_,_),
       FR(_,_,_,_,_),
       FR(_,_,X,_,_)),
    FC(FR(_,_,_,_,X),   // /
       FR(_,_,_,X,_),
       FR(_,_,X,_,_),
       FR(_,_,X,_,_),
       FR(_,X,_,_,_),
       FR(X,_,_,_,_)),
};

// sequence of extra chars at the end:
static const char pFontExtras[] PROGMEM = "()+-:@'=?/";

uint8_t font5x6_GetRow(char ch, int row)
{
  // get the row of the char, 5 bits
  int idx;
  // compute the char's index into the table
  const char* pExtra = ::strchr_P(pFontExtras, ch);
  if ('A' <= ch && ch <= 'Z')
    idx = ch - 'A';
#ifdef FONT5x6_LOWERCASE    
  else if ('a' <= ch && ch <= 'z')
    idx = ch - 'a' + 26;
  else if ('0' <= ch && ch <= '9')
    idx = ch - '0' + 26 + 26;
  else if (pExtra)  
    idx = (int)(pExtra - pFontExtras) + 26 + 26 + 10;
  #else
  else if ('a' <= ch && ch <= 'z')
    idx = ch - 'a';
  else if ('0' <= ch && ch <= '9')
    idx = ch - '0' + 26;
  else if (pExtra)  
    idx = (int)(pExtra - pFontExtras) + 26 + 10;
#endif
  else
    return 0;

  // get the whole char defn
  unsigned long defn = pgm_read_dword(pFont5x6 + idx);
  
#ifdef FONT5x6_DESCENDERS
  // adjust row for descender, if applicable
  bool descender = true;
  if (defn & DESCENDER)
    defn &= ~DESCENDER;
  else
    descender = false;
  if (descender)
  {
    if (row)
      row--; // shuffle down
    else
      return 0;
  }
#endif

  // return the row
  defn >>= (row*5);
  defn &= 0b00011111;
  return (uint8_t)defn;
}
