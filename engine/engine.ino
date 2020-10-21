//--------------------------------------------------------------------------
// Expression Engine - shape based expressions for interactive Robots
// Copyright (c) Hierophect 2019
// Inspired by the Uncanny Eyes project by Phil Burgess for Adafruit Industries
//--------------------------------------------------------------------------

#include <SPI.h>
#include <Adafruit_GFX.h>

#define JOJO            0
#define ORISA_DEMO      0
#define ORISA_DYN       1

#define TFT_SPI        SPI
#define TFT_PERIPH     PERIPH_SPI

#ifdef PIXEL_DOUBLE
  // For the 240x240 TFT, pixels are rendered in 2x2 blocks for an
  // effective resolution of 120x120. M0 boards just don't have the
  // space or speed to handle an eye at the full resolution of this
  // display (and for M4 boards, take a look at the M4_Eyes project
  // instead). 120x120 doesn't quite match the resolution of the
  // TFT & OLED this project was originally developed for. Rather
  // than make an entirely new alternate set of graphics for every
  // eye (would be a huge undertaking), this currently just crops
  // four pixels all around the perimeter.
  #define SCREEN_X_START 4
  #define SCREEN_X_END   (SCREEN_WIDTH - 4)
  #define SCREEN_Y_START 4
  #define SCREEN_Y_END   (SCREEN_HEIGHT - 4)
#else
  #define SCREEN_X_START 0
  #define SCREEN_X_END   SCREEN_WIDTH
  #define SCREEN_Y_START 0
  #define SCREEN_Y_END   SCREEN_HEIGHT
#endif

#define MAX(x,y) (((x) >= (y)) ? (x) : (y))
#define MIN(x,y) (((x) <= (y)) ? (x) : (y))

#include <Adafruit_SSD1351.h>       // OLED display library

#include "config.h"     // ****** CONFIGURATION IS DONE IN HERE ******

SPISettings settings(SPI_FREQ, MSBFIRST, SPI_MODE0);

#include "samd_SSD1351.h"

int cos_lookup[360] = {};
int sin_lookup[360] = {};

// SETUP
// Initializes the displays using pre-existing libraries, with some modifications.
// Also pre-calculates some trig data to use later.

void setup() {
    // start serial
    Serial.begin(115200);
    //while (!Serial);
    Serial.println("Init");

    display_init();

    //calculate the trig lookup tables
    uint16_t time = millis();
    for(int i=0;i<360;i++) {
        float i_fl = (float)i;
        cos_lookup[i] = cos(i_fl/57.2958)*65535;
        sin_lookup[i] = sin(i_fl/57.2958)*65535;
    }
    time = millis() - time;
    Serial.println("trig calcs:");
    Serial.println(time, DEC);
    Serial.println("Processor Speed:");
    Serial.println(F_CPU, DEC);

    // only print startup message to one eye
    uint8_t id = 0;
    displays[id]->setCursor(0, 5);
    displays[id]->setTextColor(YELLOW);
    displays[id]->setTextSize(1);
    displays[id]->println("Done");
    displays[id]->println(F_CPU, DEC);

    delay(1000);
}

// EASE ARRAY
// Used for any kind of smooth motion, slows down at beginning and end.

const uint8_t ease[] = { // Ease in/out curve for eye movements 3*t^2-2*t^3
    0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  2,  2,  2,  3,   // T
    3,  3,  4,  4,  4,  5,  5,  6,  6,  7,  7,  8,  9,  9, 10, 10,   // h
   11, 12, 12, 13, 14, 15, 15, 16, 17, 18, 18, 19, 20, 21, 22, 23,   // x
   24, 25, 26, 27, 27, 28, 29, 30, 31, 33, 34, 35, 36, 37, 38, 39,   // 2
   40, 41, 42, 44, 45, 46, 47, 48, 50, 51, 52, 53, 54, 56, 57, 58,   // A
   60, 61, 62, 63, 65, 66, 67, 69, 70, 72, 73, 74, 76, 77, 78, 80,   // l
   81, 83, 84, 85, 87, 88, 90, 91, 93, 94, 96, 97, 98,100,101,103,   // e
  104,106,107,109,110,112,113,115,116,118,119,121,122,124,125,127,   // c
  128,130,131,133,134,136,137,139,140,142,143,145,146,148,149,151,   // J
  152,154,155,157,158,159,161,162,164,165,167,168,170,171,172,174,   // a
  175,177,178,179,181,182,183,185,186,188,189,190,192,193,194,195,   // c
  197,198,199,201,202,203,204,205,207,208,209,210,211,213,214,215,   // o
  216,217,218,219,220,221,222,224,225,226,227,228,228,229,230,231,   // b
  232,233,234,235,236,237,237,238,239,240,240,241,242,243,243,244,   // s
  245,245,246,246,247,248,248,249,249,250,250,251,251,251,252,252,   // o
  252,253,253,253,254,254,254,254,254,255,255,255,255,255,255,255 }; // n

#define LOOP_RECTANGLE  0
#define PUPIL_GLYPH 0

int16_t findXIntersect(uint8_t y, int16_t x0, int16_t x1, int16_t y0, int16_t y1) {
    // Gets the value of X intersection at Y from the slope of a line. Assumes bounded.
    return (y-y0) * (x1-x0)/(y1-y0) + x0;
}

#define USE_LINE_RECTS 1

uint16_t time0, time1, time2, time3, time4;

// MAIN LOOP
// Contains a list of exchangable animations
// Animations use custom algorithms to calculate glyph parameters, like location and size.
// These parameters are mapped to the size of the screen. After calculation, draw is called,
// using the updated parameters in the array to update the screen.

#include "animations/orisa_dyn.h"
//#include "animations/orisa_static.h"
//#include "animations/jojo.h"
//#include "animations/rectangle.h"

void loop() {
    animate_frame();

    draw(0);
    // //reverse X for second eye
    // glyphs[PUPIL_GLYPH].x = 128 - glyphs[PUPIL_GLYPH].x;
    // //turbohack
    // if(cs != 6) {
    //     for (int i = 0; i < 4; i++) {
    //         glyphs[i+1].x = 128 - glyphs[i+1].x;
    //     }
    // }
    // draw(1);
    // //do it again to revert
    // glyphs[PUPIL_GLYPH].x = 128 - glyphs[PUPIL_GLYPH].x;
    // if(cs != 6) {
    //     for (int i = 0; i < 4; i++) {
    //         glyphs[i+1].x = 128 - glyphs[i+1].x;
    //     }
    // }
}

// DRAW
// This function is responsible for setting up the high-speed bulk write to the screen.
// It sets up the screen for DMA or FIFO, then calculates each pixel individually based on the
// glyph and layer data, loading it pixel by pixel into the buffer for speed, then terminates
// the transaction manually. Contains a lot of optimization, particularly for rectangles.

void draw(uint8_t eye) {
    // TIMEBLOC ---------
    //uint16_t time0 = millis();
    //uint16_t time1, time2, time3, time4;

    uint16_t p = 0;

    display_start_SPI(eye);

    uint8_t layersize = sizeof(glyphs)/sizeof(*glyphs);

    // Rect optimization - corner calcs
    for(uint8_t i = 0; i < layersize; i++) {
        //precalculate corner points
        if (glyphs[i].type == TYPE_RECT) {
            uint16_t r = glyphs[i].r.angle;
            uint16_t h = glyphs[i].r.height;
            uint16_t l = glyphs[i].r.width;
            uint16_t x0 = glyphs[i].x;
            uint16_t y0 = glyphs[i].y;

            // Not factored for mathematical clarity
            // P1:-x/+y
            glyphs[i].r.x1 = ((cos_lookup[r]*(-l/2))/65535) - ((sin_lookup[r]*(h/2))/65535) + x0;
            glyphs[i].r.y1 = ((sin_lookup[r]*(-l/2))/65535) + ((cos_lookup[r]*(h/2))/65535) + y0;
            // P2:+x/+y
            glyphs[i].r.x2 = ((cos_lookup[r]*(l/2))/65535) - ((sin_lookup[r]*(h/2))/65535) + x0;
            glyphs[i].r.y2 = ((sin_lookup[r]*(l/2))/65535) + ((cos_lookup[r]*(h/2))/65535) + y0;
            // P3:-x/-y
            glyphs[i].r.x3 = ((cos_lookup[r]*(-l/2))/65535) - ((sin_lookup[r]*(-h/2))/65535) + x0;
            glyphs[i].r.y3 = ((sin_lookup[r]*(-l/2))/65535) + ((cos_lookup[r]*(-h/2))/65535) + y0;
            // P4:+x/-y
            glyphs[i].r.x4 = ((cos_lookup[r]*(l/2))/65535) - ((sin_lookup[r]*(-h/2))/65535) + x0;
            glyphs[i].r.y4 = ((sin_lookup[r]*(l/2))/65535) + ((cos_lookup[r]*(-h/2))/65535) + y0;

        }
    }
    int16_t rect_intersections[layersize][3]; //[intersects-true, x1, x2]

    //time2 = millis() - time0;

    // Start Y "scanline"
    for(uint8_t screenY = 0; screenY < SCREEN_HEIGHT; screenY++) {
        // Start a new DMA... thing
        dma_reset_ptr();

        // Y level rectangle line detection
        #if (USE_LINE_RECTS)
        for (uint8_t i = 0; i < layersize; i++) {
            int16_t a = 0;
            bool aset = false;
            int16_t b = 0;
            bool bset = false;
            int16_t x1 = glyphs[i].r.x1;
            int16_t x2 = glyphs[i].r.x2;
            int16_t x3 = glyphs[i].r.x3;
            int16_t x4 = glyphs[i].r.x4;
            int16_t y1 = glyphs[i].r.y1;
            int16_t y2 = glyphs[i].r.y2;
            int16_t y3 = glyphs[i].r.y3;
            int16_t y4 = glyphs[i].r.y4;
            int16_t y = screenY;
            //line 1-2
            if (y <= MAX(y1,y2) && y > MIN(y1,y2)) {
                // intersection detected
                a = findXIntersect(y, x1, x2, y1, y2);
                aset = true;
            }
            //line 1-3
            if(y <= MAX(y1,y3) && y > MIN(y1,y3)) {
                if (!aset) {
                    a = findXIntersect(y, x1, x3, y1, y3);
                    aset = true;
                } else {
                    b = findXIntersect(y, x1, x3, y1, y3);
                    bset = true;
                }
            }
            //line 3-4
            if(y <= MAX(y3,y4) && y > MIN(y3,y4)) {
                if (!aset) {
                    a = findXIntersect(y, x3, x4, y3, y4);
                    aset = true;
                } else {
                    b = findXIntersect(y, x3, x4, y3, y4);
                    bset = true;
                }
            }
            //line 2-4
            if(y <= MAX(y2,y4) && y > MIN(y2,y4)) {
                if (!aset) {
                    a = findXIntersect(y, x2, x4, y2, y4);
                    aset = true;
                } else {
                    b = findXIntersect(y, x2, x4, y2, y4);
                    bset = true;
                }
            }
            // if two intersections detected, assign
            if (aset && bset) {
                rect_intersections[i][0] = true;
                rect_intersections[i][1] = MIN(a,b);
                rect_intersections[i][2] = MAX(a,b);
            } else {
                rect_intersections[i][0] = false;
            }
        }
        #endif

        // Start X "column"
        for(uint8_t screenX = 0; screenX < SCREEN_WIDTH; screenX++) {
            // pixel default is black unless changed
            p = BLACK;
            // layers are evaluated per pixel
            for(uint8_t l = 0; l < layersize; l++) {
                // pixel evaluation differs by type
                if (glyphs[l].type == TYPE_CIRCLE) {
                    // circle equation: (xp−xc)^2+(yp−yc)^2 < r^2
                    int dsqr = (screenX - glyphs[l].x) * (screenX - glyphs[l].x)
                                + (screenY - glyphs[l].y) * (screenY - glyphs[l].y);
                    if (dsqr < (glyphs[l].c.radius * glyphs[l].c.radius)) {
                        p = glyphs[l].c.color;
                    }
                } else if (glyphs[l].type == TYPE_RECT) {
                    #if (USE_LINE_RECTS)
                    if(rect_intersections[l][0]) {
                        if (screenX > rect_intersections[l][1] && screenX < rect_intersections[l][2]) {
                            p = glyphs[l].r.color;
                            // if (glyphs[l].masking) {
                            //     goto ESCAPE;
                            // }
                        }
                        // if (screenX == rect_intersections[l][1] || screenX == rect_intersections[l][2]) {
                        //     p = glyphs[l].r.color;
                        // }
                    }
                    #else
                    //--- point-by-point approach (worse framerates)
                    // transform point shift
                    int xshi = screenX - glyphs[l].x;
                    int yshi = screenY - glyphs[l].y;
                    // rotate point into box transform
                    int x2 = (cos_lookup[glyphs[l].r.angle]*(xshi)) / 65535 -
                                (sin_lookup[glyphs[l].r.angle] * (yshi)) / 65535 + glyphs[l].x;
                    int y2 = (sin_lookup[glyphs[l].r.angle]*(xshi)) / 65535 +
                                (cos_lookup[glyphs[l].r.angle] * (yshi)) / 65535 + glyphs[l].y;
                    if (x2 < (glyphs[l].x + (glyphs[l].r.width / 2))
                            && x2 > (glyphs[l].x - (glyphs[l].r.width/2))
                            && y2 < (glyphs[l].y + (glyphs[l].r.height/2))
                            && y2 > (glyphs[l].y - (glyphs[l].r.height/2))) {
                        p = glyphs[l].r.color;
                    }
                    #endif

                } else if (glyphs[l].type == TYPE_IMAGE) {
                    // Get multidimensional array from void pointer
                    typedef const uint16_t aType[glyphs[l].i.height][glyphs[l].i.width];
                    aType *c = (aType*) glyphs[l].i.array;

                    if (screenX < (glyphs[l].x + (glyphs[l].i.width / 2))
                            && screenX > (glyphs[l].x - (glyphs[l].i.width/2))
                            && screenY < (glyphs[l].y + (glyphs[l].i.height/2))
                            && screenY > (glyphs[l].y - (glyphs[l].i.height/2))) {
                        uint16_t temp = (*c)[screenY - (glyphs[l].y - (glyphs[l].i.height/2))]
                                [screenX - (glyphs[l].x - (glyphs[l].i.width/2))];
                        if (temp == 0xFFFF) {
                            continue;
                        } else {
                            p = temp;
                        }
                    }
                }
            }
            // Load the pixel with fifo or DMA
            display_load_pixel(p);

        } // end column
        update_DMA(eye, screenY);
    } // end scanline

    //time3 = millis() - time0;

    display_end_SPI(eye);


    //TIMEBLOC ---------
    //time4 = millis() - time0;

    //Serial.printf("Frame: t1:%d, t2:%d, t3:%d, t4:%d\n", time1, time2, time3, time4);
}

