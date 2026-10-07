#!/usr/bin/python
import sys
import argparse
import os
from PIL import Image
# convert Moon image to progmem data

# each line is 
# 1 byte N=number of non-black pixels   
# N/2 bytes two pixels per bytes, 0..15 gray scale
# 
# so if W=width, (W-N/2) black pix, N pix, (W-N/2) black pix


def ByteStr(b):
    return "0x" + hex(256 + b)[3:].upper()

def Encode(upright):
    if upright:
        name = "pMoonImageRowsNorth"
    else:
        name = "pMoonImageRowsSouth"
    bytes = 0
    file.write("static const uint8_t " + name + "[] PROGMEM =\n")
    file.write("{\n")
    file.write("  // number of black pixels at row start, colour pixels (two per byte, grey scale, 0xRL)...\n")

    for y in range(rgb.height):
        blackCount = 0
        prevVal = -1
        colourPixels = 0
        for x in range(rgb.width):
            if upright:
                val = rgb.getpixel((x, y))[0] # just the red
            else:
                val = rgb.getpixel((rgb.width - x - 1, rgb.height - y - 1))[0] # just the red
            if val <= 1:
                blackCount += 1
            else:
                if x == 0 or blackCount != 0:
                    file.write("  " + ByteStr(blackCount) + ",  ")
                    bytes += 1
                    for indent in range(blackCount):
                        file.write("  "),
                    colourPixels = rgb.width - 2*blackCount
                    blackCount = 0
                # fit the range to the palette (16 colours incl black & white)
                div = (maxVal - minVal)/15
                val = int(round(((val - minVal)/div)))
                #val = int(val/16)
                colourPixels -= 1
                if colourPixels < 0:
                    break
                if prevVal == -1:
                    prevVal = val
                else:
                    file.write(ByteStr(prevVal + val * 16) + ",")
                    prevVal = -1
                    bytes += 1
        if blackCount == rgb.width: # empty row
            file.write("  " + ByteStr(blackCount) + ",")
            bytes += 1
        if prevVal != -1:
            file.write(ByteStr(prevVal) + ",")
            bytes += 1
            colourPixels -= 1
        while colourPixels > 0:
            colourPixels -= 2
            file.write("0x00,")
            bytes += 1
        file.write("\n")
    file.write("}; // " + str(bytes)+ " bytes\n\n\n")

if len(sys.argv) != 2:
    print("Missing moon bmp filename!")
    exit()
bmp = Image.open(sys.argv[1])
rgb = bmp.convert('RGB')

# get the grayscale range
minVal = 255
maxVal = 0
for y in range(rgb.height):
    for x in range(rgb.width):
        val = rgb.getpixel((x, y))[0] # just the red
        minVal = min(val, minVal)
        maxVal = max(val, maxVal)
#print("min %s max %s" % (minVal, maxVal))

file = open("MoonData.h", "w")
file.write("// (created from \"" + bmp.filename + "\" by " + os.path.basename(__file__) + ")\n")
file.write("#define MOON_IMAGE_WIDTH  " + str(rgb.width) + "\n")
file.write("#define MOON_IMAGE_HEIGHT " + str(rgb.height) + "\n\n")
Encode(True)
Encode(False)
file.close()
print("Created %s" % file.name)



