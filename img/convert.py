# Copy pasted code from Convert. Works file by file. 

import sys
import math
from PIL import Image
from hextable import HexTable

# OPEN AND VALIDATE SCLERA IMAGE FILE --------------------------------------

try:
    FILENAME = sys.argv[1]
except IndexError:
    FILENAME = 'kirbo.png' # Default filename if argv 1 not provided
IMAGE = Image.open(FILENAME)
IMAGE = IMAGE.convert('RGB')
PIXELS = IMAGE.load()

# GENERATE IMAGE ARRAY ----------------------------------------------------

print('#define IMAGE_WIDTH  ' + str(IMAGE.size[0]))
print('#define IMAGE_HEIGHT ' + str(IMAGE.size[1]))
print('')

sys.stdout.write('const uint16_t image[IMAGE_HEIGHT][IMAGE_WIDTH] = {')
HEX = HexTable(IMAGE.size[0] * IMAGE.size[1], 8, 4)

# Convert 24-bit image to 16 bits:
for y in range(IMAGE.size[1]):
    for x in range(IMAGE.size[0]):
        p = PIXELS[x, y] # Pixel data (tuple)
        HEX.write(
            ((p[0] & 0b11111000) << 8) | # Convert 24-bit RGB
            ((p[1] & 0b11111100) << 3) | # to 16-bit value w/
            (p[2] >> 3))                 # 5/6/5-bit packingpython converter