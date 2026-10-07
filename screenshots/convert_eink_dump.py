#!/usr/bin/python
import sys
import argparse
import os
from PIL import Image

# converts image dump from QuotidianMoon (EINK_DUMP_IMAGE) to a PNG
# format is
#  I                (optional, if display is inverted)
#  M                (mono)
#  XXXX......XXXX   (100 chars, 50 hex bytes, 0b11=white, 0b10=grey, 0b00=black, 4 pixels per byte, leftmost pixel is upper bits)
#  XXXX......XXXX
#  ...
#  XXXX...XXXX
#  R                (red)
#  XXXX...XXXX      (50 chars, 25 hex bytes, 0b0=red, 0b1= N/A, 8 pixels per byte, leftmost pixel is upper bit)
#  XXXX...XXXX
#  ...
#  XXXX...XXXX

def Coords(col, row):
    global invert
    if invert:
        return (200 - col - 1, 200 - row - 1)
    else:
        return (col, row)
    
if len(sys.argv) != 2:
    print 'convert_eink_dump.py <inputfile>'
    sys.exit(2)    
fileName = sys.argv[1]

dump = open(fileName, 'r')

bmp = Image.new("RGB", (200,200), "black")
line = dump.readline();
lineNum = 1
row = 0
mono = True
invert = False
while line:
    line = line.strip()
    items = [line[i:i+2] for i in range(0, len(line), 2)]
    if line[0] == ';':
        # ignore comment
        pass
    elif line[0] == 'M':
        mono = True
        row = 0
        pass
    elif line[0] == 'R':
        row = 0
        mono = False
        pass
    elif line[0] == 'I':
        invert = True
        pass
    elif len(items) > 1:
        col = 0
        for item in items:
            if item != " ":
                byte = int(item, 16)
                if mono:
                    for pix in range(4):
                        palette = byte & 0b11000000 # black
                        byte = byte << 2
                        rgb = (0, 0, 0)
                        if palette == 0b10000000:
                            rgb = (230, 230, 230) # grey is very light
                        elif palette == 0b11000000:
                            rgb = (255, 255, 255) # white
                        bmp.putpixel(Coords(col, row), rgb)
                        col += 1
                else:
                    for pix in range(8):
                        palette = byte & 0b10000000
                        byte = byte << 1
                        if palette == 0:
                            bmp.putpixel(Coords(col, row), (200, 0, 0)) # red is quite dark
                        col += 1
        row += 1
    line = dump.readline();
    lineNum = lineNum + 1

bmp.save(os.path.splitext(fileName)[0] + ".png")
dump.close()
