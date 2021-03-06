#include <SPI.h>
#include <Adafruit_GFX.h>

#define TFT_SPI        SPI
#define TFT_PERIPH     PERIPH_SPI

#include <Adafruit_SSD1351.h>       // OLED display library

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   128         // Change this to 96 for 1.27" OLED.
#define DISPLAY_DC      7           // Data/command pin for ALL displays
#define DISPLAY_RESET   8           // Reset pin for ALL displays
#define DISPLAY_SEL1    9
#define DISPLAY_SEL2    10  
#define SPI_FREQ        24000000    // OLED: 24 MHz on 72 MHz Teensy only (am I overclocking?)
// #define SPI_FREQ        12000000    // OLED: 24 MHz on 72 MHz Teensy only (am I overclocking?)

// Color definitions
#define BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

#define TYPE_CIRCLE     0
#define TYPE_RECT       1

//create both screens
Adafruit_SSD1351 disp1 = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &TFT_SPI,
                        DISPLAY_SEL1, DISPLAY_DC, -1);
Adafruit_SSD1351 disp2 = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &TFT_SPI,
                        DISPLAY_SEL2, DISPLAY_DC, -1);
// Adafruit_SSD1351 displays[] = {
//     Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &TFT_SPI,
//                         DISPLAY_SEL1, DISPLAY_DC, -1),
//     Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &TFT_SPI,
//                         DISPLAY_SEL2, DISPLAY_DC, -1)
// };
Adafruit_SSD1351 displays[] = {disp1, disp2};
uint8_t select_pins[] = {DISPLAY_SEL1, DISPLAY_SEL2};

SPISettings settings(SPI_FREQ, MSBFIRST, SPI_MODE0);

int cos_lookup[360] = {};
int sin_lookup[360] = {};

typedef struct {
    uint16_t radius;
    uint16_t color;
} circleGlyph;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t angle;
    uint16_t color;
} rectGlyph;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t type;
    union
    {
        circleGlyph c;
        rectGlyph r;
    };
} eyeGlyph;

eyeGlyph glyphs[] = {
    {64, 64, TYPE_CIRCLE, { .c = {40, YELLOW} }},
    // {4, 4, TYPE_RECT, { .r = {255,32,0, BLACK} }},
    // {4, 124, TYPE_RECT, { .r = {32,255,0, BLACK} }},
    // {124, 4, TYPE_RECT, { .r = {32,255,0, BLACK} }},
    // {124, 124, TYPE_RECT, { .r = {255,32,0, BLACK} }},

    //{64, 64, TYPE_RECT, { .r = {32,128,90, RED} }},
};

// Examples:
// {64, 64, TYPE_CIRCLE, { .c = {32, RED} }},
// {64, 64, TYPE_RECT, { .r = {32,128,90, RED} }},

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
    disp2.sendCommand(SSD1351_CMD_SETREMAP, &rotateOLED[0], 1); //no mirror
    //disp2.sendCommand(SSD1351_CMD_SETREMAP, &mirrorOLED[0], 1);
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

void loop() {
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

    eyeX = map(eyeX, 0, 1023, 0, 128);
    eyeY = map(eyeY, 0, 1023, 0, 128);

    uint16_t evil = map(eyeR, 0, 1023, 0, 25);

    uint16_t altcolor = 0xF800 | ((0x3F - (0x02 * evil)) << 5) | (0x1F - (0x01 *evil));
    glyphs[0].c.color = altcolor;
    glyphs[0].c.radius = 40-evil;

    glyphs[0].x = eyeX;
    glyphs[0].y = eyeY;

    draw(0);
    draw(1);

}

void draw(uint8_t eye) {
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

    //manually start data mode
    digitalWrite(select_pins[eye], LOW);                // Re-chip-select
    digitalWrite(DISPLAY_DC, HIGH);                      // Data mode

    uint8_t layersize = sizeof(glyphs)/sizeof(*glyphs);

    //TIMEBLOC ---------
    uint16_t time = millis();

    for(uint8_t screenY = 0; screenY < SCREEN_HEIGHT; screenY++) {
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
                    // transform point shift
                    int xshi = screenX - glyphs[l].x;
                    int yshi = screenY - glyphs[l].y;
                    // rotate point into box transform
                    int x2 = (cos_lookup[glyphs[l].r.angle]*(xshi)) / 65535 + 
                                (sin_lookup[glyphs[l].r.angle] * (yshi)) / 65535 + glyphs[l].x;
                    int y2 = (-sin_lookup[glyphs[l].r.angle]*(xshi)) / 65535 + 
                                (cos_lookup[glyphs[l].r.angle] * (yshi)) / 65535 + glyphs[l].y;
                    if (x2 < (glyphs[l].x + (glyphs[l].r.width / 2)) 
                            && x2 > (glyphs[l].x - (glyphs[l].r.width/2)) 
                            && y2 < (glyphs[l].y + (glyphs[l].r.height/2)) 
                            && y2 > (glyphs[l].y - (glyphs[l].r.height/2))) {
                        p = glyphs[l].r.color;
                    }
                }
            }
            
            // SPI FIFO technique from Paul Stoffregen's ILI9341_t3 library:
            while(KINETISK_SPI0.SR & 0xC000); // Wait for space in FIFO
            KINETISK_SPI0.PUSHR = p | SPI_PUSHR_CTAS(1) | SPI_PUSHR_CONT;
        } // end column
    } // end scanline

    time = millis() - time;

    //manually end SPI transaction
    KINETISK_SPI0.SR |= SPI_SR_TCF;         // Clear transfer flag
    while((KINETISK_SPI0.SR & 0xF000) ||    // Wait for SPI FIFO to drain
             !(KINETISK_SPI0.SR & SPI_SR_TCF)); // Wait for last bit out

    digitalWrite(select_pins[eye], HIGH);          // Deselect
    TFT_SPI.endTransaction();

    //TIMEBLOC ---------
    
    Serial.println("Loop:");
    Serial.println(time, DEC);
}
