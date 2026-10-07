#!/usr/bin/python
import sys
import argparse
import os
from PIL import Image
# convert Splash top half of 32x128 image to progmem data
# alternating black/white run lengths



def ByteStr(b):
    return "0x" + hex(256 + b)[3:].upper()


file = open("SplashData.h", "w")

def Encode(north):
    bytes = 0
    if north:
        hemi = "North"
        bmp = Image.open("SplashN.bmp")
    else:
        hemi = "South"
        bmp = Image.open("SplashS.bmp")
    rgb = bmp.convert('RGB')
    file.write("// (created from \"" + bmp.filename + "\" by " + os.path.basename(__file__) + ")\n")
    file.write("static const uint8_t pSplashImageData" + hemi + "[] PROGMEM =\n")
    file.write("{  // <#black>, <#white>, <#black>, ...\n")
    file.write("   // (only the top half of image)\n")

    for y in range(rgb.height/2):
        black = True
        count = 0
        file.write("  ")
        for x in range(rgb.width):
            if rgb.getpixel((x, y))[0] < 64: # black
                if black:
                    count += 1 # extend
                else: # switch
                    file.write(ByteStr(count) + ", ") # white count
                    black = True
                    bytes += 1
                    count = 1
            else: # white
                if black:
                    file.write(ByteStr(count) + ", ") # black count
                    black = False
                    bytes += 1
                    count = 1
                else:
                    count += 1
        file.write(ByteStr(count) + ", ")
        bytes += 1
        count = 0
        file.write("\n")
    file.write("}; // " + str(bytes)+ " bytes\n\n")

Encode(True)
file.write("\n\n")
Encode(False)
file.close()
print("Created %s" % file.name)



