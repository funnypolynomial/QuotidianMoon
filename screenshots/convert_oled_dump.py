#!/usr/bin/python
import sys
import argparse
import os
from PIL import Image

# converts image dump from QuotidianMoon (OLED_DUMP_IMAGE) to a PNG
# format is
#  P <page> <start-col n> <col-data n> <col-data n+1> <col-data n+2>...
# Set column bytes at page, staring at column. lsb is top
    
if len(sys.argv) != 2:
    print 'convert_oled_dump.py <inputfile>'
    sys.exit(2)    
fileName = sys.argv[1]

dump = open(fileName, 'r')

bmp = Image.new("RGB", (128,32), "black")
line = dump.readline();
lineNum = 1
while line:
    line = line.strip()
    items = line.split()
    if line[0] == ';':
        # ignore comment
        pass
    elif line[0] == 'P'and len(items) > 3:
        page = int(items[1])*8
        col = int(items[2])
        items = items[3:]
        for item in items:
            col_data = int(item)
            for bit in range(8):
                if col_data & (1 << bit):
                    bmp.putpixel((col, page + bit), (255, 255, 255))
                else:
                    bmp.putpixel((col, page + bit), (0, 0, 0))
            col += 1
    line = dump.readline();
    lineNum = lineNum + 1

bmp.save(os.path.splitext(fileName)[0] + ".png")
dump.close()
