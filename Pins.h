#pragma once

// Schematic
// (FINAL, non-prototype)
//
//        +-----------------+
//  [SDA]-+(SDA)  <1>  (SCL)+-[SCL]
//        |                 |      
// [BUSY]-+A0    Leo     ~D9+-[CS] 
//        |      Tiny       |      
// [BTNS]-+A1           ~D10+-[DC] 
//        |                 |      
//  [LDR]-+A2           ~D11+-[RST]
//        |                 |      
// [5VDC]-+"+"           "0"+-[GND]
//        |                 |      
// [MOSI]-+(MOSI) <2>  (SCK)+-[SCK]
//        | D16         D15 |
//        +------|USB|------+
//
//
//        +-----------------------------------+ 
// [5VDC]-+VIN                                | 
//  [GND]-+GND         OLED 32x128            |
//  [SCL]-+SCL                                | 
//  [SDA]-+SDA                                | 
//        +-----------------------------------+ 
//
//
//        +-----------------------+
//        |                       |
//        |                   BUSY+-[BUSY] 
//        |                    RST+-[RST] 
//        |                     DC+-[DC] 
//        |   eInk Display      CS+-[CS]  
//        |     200x200        CLK+-[SCK]
//        |                    DIN+-[MOSI]
//        |                    GND+-[GND]
//        |                    VCC+-[5VDC]
//        |                       |
//        +-----------------------+
//
//
//             +---------+       +-------+
//    [5VDC]---+ LDR <3> +---+---| 120kR |---[GND]
//             +---------+   |   +-------+
//                           |
//                           +---[LDR]
//
//
//                 SET <4>
//                 ---
//             +---   -------------------+
//             |                         |
//             |              SEL <4>    |
//             |  +------+    ---        |          +------+
//     [5VDC]--+--| 220R |----   --------+----------| 220R |---[GND]
//                +------+               |          +------+
//                                       +---[BTNS]
//
//
//
// Notes:
// Leonardo Tiny: https://www.jaycar.co.nz/leonardo-tiny-atmega32u4-main-board/p/XC4431
// eInk Display: https://www.jaycar.co.nz/duinotech-arduino-compatible-1-54-inch-monochrome-e-ink-display/p/XC3747
// OLED Display: 128x32 0.91 inch. Pins are GND|VCC|SCL|SDA. SSD1306
// Matching [LABELS] are connected.
//  <1>: SDA & SCL are pads on the underside of the Leo
//  <2>: MOSI, SCK & MISO are ICSP pads on the underside of the Leo
//  <3>: LDR is ~200R bright, ~20MR dark 
//  <4>: Two push-buttons (N/O), https://www.jaycar.co.nz/spst-pcb-tactile-switch/p/SP0611
//       BTNS pin reads ~0V with no button pressed, ~5V with SET pressed, ~2.5V with SEL pressed


#if defined(ARDUINO_AVR_LEONARDO_ETH) || defined(ARDUINO_AVR_LEONARDO)
// Leo Tiny
// eInk:
#define PIN_DISPLAY_BUSY  9
#define PIN_DISPLAY_RST   10
#define PIN_DISPLAY_DC    11
#define PIN_DISPLAY_CS    A2

#define PIN_BTNS          A1
#define PIN_LDR           A0

// (SPI)
#define PIN_DISPLAY_SCK   15
#define PIN_DISPLAY_MOSI  16

#else
board not implemented!
#endif
