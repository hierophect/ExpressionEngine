//--------------------------------------------------------------------------
// Expression Engine - shape based expressions for interactive Robots
// Copyright (c) Hierophect 2019
// Inspired by the Uncanny Eyes project by Phil Burgess for Adafruit Industries
//--------------------------------------------------------------------------

#include <SPI.h>
#include <Adafruit_GFX.h>

#define JOJO    0
#define ORISA   1

#define TFT_SPI        SPI
#define TFT_PERIPH     PERIPH_SPI

#define MAX(x,y) (((x) >= (y)) ? (x) : (y))
#define MIN(x,y) (((x) <= (y)) ? (x) : (y))

#include <Adafruit_SSD1351.h>       // OLED display library

#include "config.h"     // ****** CONFIGURATION IS DONE IN HERE ******

//create both screens
Adafruit_SSD1351 disp1 = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &TFT_SPI,
                        DISPLAY_SEL1, DISPLAY_DC, -1);
Adafruit_SSD1351 disp2 = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &TFT_SPI,
                        DISPLAY_SEL2, DISPLAY_DC, -1);

Adafruit_SSD1351 displays[] = {disp1, disp2};
uint8_t select_pins[] = {DISPLAY_SEL1, DISPLAY_SEL2};

SPISettings settings(SPI_FREQ, MSBFIRST, SPI_MODE0);

int cos_lookup[360] = {};
int sin_lookup[360] = {};

void setup() {
    // start serial
    Serial.begin(115200);
    //while (!Serial);
    Serial.println("Init");

    // manual reset since they share reset line
    Serial.println("Reset displays");
    pinMode(DISPLAY_RESET, OUTPUT);
    digitalWrite(DISPLAY_RESET, LOW);  delay(1);
    digitalWrite(DISPLAY_RESET, HIGH); delay(50);

    // hold sel high for both screens
    pinMode(DISPLAY_SEL1, OUTPUT);
    digitalWrite(DISPLAY_SEL1, HIGH);  
    pinMode(DISPLAY_SEL2, OUTPUT);
    digitalWrite(DISPLAY_SEL2, HIGH); 

    // disp1 setup
    disp1.begin();
    disp1.setRotation(0);
    disp1.fillScreen(BLACK);
    // disp2 setup
    disp2.begin();
    disp2.setRotation(0);
    disp2.fillScreen(BLACK);
    Serial.println("Init done");

    // only print debug to one eye
    disp1.setCursor(0, 5);
    disp1.setTextColor(YELLOW);  
    disp1.setTextSize(1);
    disp1.println("Calculating Lookup");

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
    disp1.fillScreen(BLACK);
    disp1.setCursor(0, 5);
    disp1.setTextColor(YELLOW);  
    disp1.setTextSize(1);
    disp1.println("Done");

    //set rotation & mirroring
    //I'd condense this but it takes pointers, and I might want to change later.
    const uint8_t rotateOLED[] = { 0x74, 0x77, 0x66, 0x65 },
                mirrorOLED[] = { 0x76, 0x67, 0x64, 0x75 }; // Mirror+rotate
    disp1.sendCommand(SSD1351_CMD_SETREMAP, &rotateOLED[0], 1); 
    //disp2.sendCommand(SSD1351_CMD_SETREMAP, &rotateOLED[0], 1); //no mirror
    disp2.sendCommand(SSD1351_CMD_SETREMAP, &mirrorOLED[0], 1);
    delay(1000);
}

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

uint16_t time0, time1, time2, time3, time4;

void loop() {

    #if JOJO
        time0 = millis();

        int16_t         eyeX, eyeY, eyeR;
        uint32_t        t = micros(); // Time at start of function

        static boolean  eyeInMotion      = false;
        static int16_t  eyeOldX=512, eyeOldY=512, eyeOldR=512, eyeNewX=512, eyeNewY=512, eyeNewR=512;
        static uint32_t eyeMoveStartTime = 0L;
        static int32_t  eyeMoveDuration  = 0L;

        int32_t dt = t - eyeMoveStartTime;      // uS elapsed since last eye event
        if(eyeInMotion) {                       // Currently moving?
            if(dt >= eyeMoveDuration) {           // Time up?  Destination reached.
                eyeInMotion      = false;           // Stop moving
                eyeMoveDuration  = random(3000000); // 0-3 sec stop
                eyeMoveStartTime = t;               // Save initial time of stop
                eyeX = eyeOldX = eyeNewX;           // Save position
                eyeY = eyeOldY = eyeNewY;
                eyeR = eyeOldR = eyeNewR;
            } else { // Move time's not yet fully elapsed -- interpolate position
                int16_t e = ease[255 * dt / eyeMoveDuration] + 1;   // Ease curve
                eyeX = eyeOldX + (((eyeNewX - eyeOldX) * e) / 256); // Interp X
                eyeY = eyeOldY + (((eyeNewY - eyeOldY) * e) / 256); // and Y
                eyeR = eyeOldR + (((eyeNewR - eyeOldR) * e) / 256); // and Y
            }
        } else {                                // Eye stopped
            eyeX = eyeOldX;
            eyeY = eyeOldY;
            eyeR = eyeOldR;
            if(dt > eyeMoveDuration) {            // Time up?  Begin new move.
                int16_t  dx, dy;
                uint32_t d;
                do {                                // Pick new dest in circle
                    eyeNewX = random(1024);
                    eyeNewY = random(1024);
                    eyeNewR = random(1024);
                    dx      = (eyeNewX * 2) - 1023;
                    dy      = (eyeNewY * 2) - 1023;
                } while((d = (dx * dx + dy * dy)) > (1023 * 1023)); // Keep trying
                eyeMoveDuration  = random(72000, 144000); // ~1/14 - ~1/7 sec
                eyeMoveStartTime = t;               // Save initial time of move
                eyeInMotion      = true;            // Start move on next frame
            }
        }

        eyeX = map(eyeX, 0, 1023, 50, 80);
        eyeY = map(eyeY, 0, 1023, 55, 80);

        // Set eye size and color
        // uint16_t evil = map(eyeR, 0, 1023, 0, 25);
        // uint16_t altcolor = 0xF800 | ((0x3F - (0x02 * evil)) << 5) | (0x1F - (0x01 *evil));
        //glyphs[PUPIL_GLYPH].c.color = altcolor;
        //glyphs[PUPIL_GLYPH].c.radius = 40-evil;

        //time1 = millis() - time0;

        // Set eye location
        glyphs[PUPIL_GLYPH].x = eyeX;
        glyphs[PUPIL_GLYPH].y = eyeY;
        draw(1);
        glyphs[PUPIL_GLYPH].x = 128 - eyeX;
        draw(0);
    #elif LOOP_RECTANGLE
    // Set rotation of test rectangle
        if (glyphs[0].r.angle < 359) {
            glyphs[0].r.angle += 2;
            glyphs[1].r.angle += 2;
            glyphs[2].r.angle += 2;
            glyphs[3].r.angle += 2;
        } else {
            glyphs[0].r.angle = 0;
            glyphs[1].r.angle = 0;
            glyphs[2].r.angle = 0;
            glyphs[3].r.angle = 0;
        }
        draw(0);
        draw(1);
    #elif ORISA
        time0 = millis();
        uint32_t t = micros();

        //statics
        static boolean  inMotion      = false;
        static uint32_t stateStartTime = 0L;
        static int32_t  stateDuration  = 0L;
        static uint8_t cs = 0; //current state

        uint8_t statesize = sizeof(states)/sizeof(*states);

        int32_t dt = t - stateStartTime;      // uS elapsed since last state change
        //Serial.printf("dt:%d, dur:%d, SS:%d, curS: %d\n", dt, stateDuration, statesize, currentState);

        static demoStates st = states[0];
        static demoStates stOld = states[0];

        if(inMotion) {                       // Currently moving?
            if(dt >= stateDuration) {           // Time up?  Destination reached.
                inMotion      = false;           // Stop moving
                stateDuration = st.duration;    //stop for set time
                stateStartTime = t;

                glyphs[PUPIL_GLYPH].x = st.pupilX;
                glyphs[PUPIL_GLYPH].y = st.pupilY;
                glyphs[PUPIL_GLYPH].c.radius = st.pupilR;

                //Rectangle positions calculated by x and y components. Angles always offset by 45
                for (int i = 0; i < 4; i++) {
                    glyphs[i+1].x = st.pupilX + ((st.d[i]*cos_lookup[(st.angle + (i*90)) % 359])/65535) + st.xoff[i];
                    glyphs[i+1].y = st.pupilY - ((st.d[i]*sin_lookup[(st.angle + (i*90)) % 359])/65535) + st.yoff[i];
                    glyphs[i+1].r.angle = ((i*90) + 45 - st.angle + 360) % 359;
                }

                // eyeX = eyeOldX = eyeNewX;           // Save position
                // eyeY = eyeOldY = eyeNewY;
                // eyeR = eyeOldR = eyeNewR;
            } else { // Move time's not yet fully elapsed -- interpolate position
                int16_t e = ease[255 * dt / stateDuration] + 1;   // Ease curve
                uint16_t intX = stOld.pupilX + (((st.pupilX - stOld.pupilX) * e) / 256); // Interp X
                uint16_t intY = stOld.pupilY + (((st.pupilY - stOld.pupilY) * e) / 256); // and Y
                uint16_t intR = stOld.pupilR + (((st.pupilR - stOld.pupilR) * e) / 256); // and Y
                uint16_t intAngle = stOld.angle + (((st.angle - stOld.angle) * e) / 256);

                glyphs[PUPIL_GLYPH].x = intX; // Interp X
                glyphs[PUPIL_GLYPH].y = intY; // and Y
                glyphs[PUPIL_GLYPH].c.radius = intR; // and Y

                //Rectangle positions calculated by x and y components. Angles always offset by 45

                for (int i = 0; i < 4; i++) {
                    glyphs[i+1].x = intX + ((st.d[i]*cos_lookup[(intAngle + (i*90)) % 359])/65535) + st.xoff[i];
                    glyphs[i+1].y = intY - ((st.d[i]*sin_lookup[(intAngle + (i*90)) % 359])/65535) + st.yoff[i];
                    glyphs[i+1].r.angle = ((i*90) + 45 - intAngle + 360) % 359;
                }
            }
        } else {                                // Eye stopped
            glyphs[PUPIL_GLYPH].x = st.pupilX;
            glyphs[PUPIL_GLYPH].y = st.pupilY;
            glyphs[PUPIL_GLYPH].c.radius = st.pupilR;

            //Rectangle positions calculated by x and y components. Angles always offset by 45
            for (int i = 0; i < 4; i++) {
                glyphs[i+1].x = st.pupilX + ((st.d[i]*cos_lookup[(st.angle + (i*90)) % 359])/65535) + st.xoff[i];
                glyphs[i+1].y = st.pupilY - ((st.d[i]*sin_lookup[(st.angle + (i*90)) % 359])/65535) + st.yoff[i];
                glyphs[i+1].r.angle = ((i*90) + 45 - st.angle + 360) % 359;
            }

            if(dt > stateDuration) {            // Time up?  Begin new move.
                stateDuration  = 344000; // ~1/7 sec
                stateStartTime = t; 
                inMotion    = true;            // Start move on next frame

                //change the state
                cs += 1;
                if (cs >= statesize) {
                    cs = 0;
                }
                stOld = st;
                st = states[cs];
            }
        }

        draw(0);
        //reverse X for second eye
        glyphs[PUPIL_GLYPH].x = 128 - glyphs[PUPIL_GLYPH].x;
        //turbohack
        if(cs != 6) {
            for (int i = 0; i < 4; i++) {
                glyphs[i+1].x = 128 - glyphs[i+1].x;
            }
        }
        draw(1);
        //do it again to revert
        glyphs[PUPIL_GLYPH].x = 128 - glyphs[PUPIL_GLYPH].x;
        if(cs != 6) {
            for (int i = 0; i < 4; i++) {
                glyphs[i+1].x = 128 - glyphs[i+1].x;
            }
        }
    #endif
}

int16_t findXIntersect(uint8_t y, int16_t x0, int16_t x1, int16_t y0, int16_t y1) {
    // Gets the value of X intersection at Y from the slope of a line. Assumes bounded. 
    return (y-y0) * (x1-x0)/(y1-y0) + x0;
}

#define USE_LINE_RECTS 1

void draw(uint8_t eye) {
    // TIMEBLOC ---------
    //uint16_t time0 = millis();
    //uint16_t time1, time2, time3, time4;

    uint16_t p = 0; 

    //manually start transaction (OLED)
    TFT_SPI.beginTransaction(settings);
    digitalWrite(select_pins[eye], LOW);
    displays[eye].writeCommand(SSD1351_CMD_SETROW);    // Y range
    displays[eye].spiWrite(0); 
    displays[eye].spiWrite(SCREEN_HEIGHT - 1);
    displays[eye].writeCommand(SSD1351_CMD_SETCOLUMN); // X range
    displays[eye].spiWrite(0); 
    displays[eye].spiWrite(SCREEN_WIDTH  - 1);
    displays[eye].writeCommand(SSD1351_CMD_WRITERAM);  // Begin write

    // Manually start data mode
    digitalWrite(select_pins[eye], LOW);                // Re-chip-select
    digitalWrite(DISPLAY_DC, HIGH);                      // Data mode

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

    for(uint8_t screenY = 0; screenY < SCREEN_HEIGHT; screenY++) {
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
            
            // Masking re-entry
            // ESCAPE:

            // SPI FIFO technique from Paul Stoffregen's ILI9341_t3 library:
            while(KINETISK_SPI0.SR & 0xC000); // Wait for space in FIFO
            KINETISK_SPI0.PUSHR = p | SPI_PUSHR_CTAS(1) | SPI_PUSHR_CONT;
        } // end column
    } // end scanline

    //time3 = millis() - time0;

    //manually end SPI transaction
    KINETISK_SPI0.SR |= SPI_SR_TCF;         // Clear transfer flag
    while((KINETISK_SPI0.SR & 0xF000) ||    // Wait for SPI FIFO to drain
             !(KINETISK_SPI0.SR & SPI_SR_TCF)); // Wait for last bit out

    digitalWrite(select_pins[eye], HIGH);          // Deselect
    TFT_SPI.endTransaction();

    //TIMEBLOC ---------
    //time4 = millis() - time0;
    
    //Serial.printf("Frame: t1:%d, t2:%d, t3:%d, t4:%d\n", time1, time2, time3, time4);
}

