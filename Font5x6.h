#pragma once

// If defined, font includes lowercase chars
#define FONT5x6_LOWERCASE
// If defined, font includes lowercase chars with descenders
#define FONT5x6_DESCENDERS

// row is 0..5 (or 6 if descenders). row 0 is topmost
// returns 0b000pqrst where p (msb) is leftmost pixel
uint8_t font5x6_GetRow(char ch, int row);
