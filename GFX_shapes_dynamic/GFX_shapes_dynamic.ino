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
#define SPI_FREQ        12000000    // OLED: 24 MHz on 72 MHz Teensy only (am I overclocking?)

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
    uint8_t radius;
    uint16_t color;
} circleGlyph;

typedef struct {
    uint8_t width;
    uint8_t height;
    uint16_t angle;
    uint16_t color;
} rectGlyph;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t type;
    union
    {
        circleGlyph c;
        rectGlyph r;
    };
} eyeGlyph;

eyeGlyph glyphs[] = {
    {64, 64, TYPE_CIRCLE, { .c = {32, WHITE} }},
    {64, 64, TYPE_RECT, { .r = {32,128,90, RED} }},
};

void setup() {
    // start serial
    Serial.begin(115200);
    while (!Serial);
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
    disp2.sendCommand(SSD1351_CMD_SETREMAP, &mirrorOLED[0], 1);
    delay(1000);
}

void loop() {
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
                    if (x2 < (glyphs[l].x + (glyphs[l].r.width/2)) 
                            && x2 > (glyphs[l].x - (glyphs[l].r.width/2)) 
                            && y2 < (glyphs[l].y + (glyphs[l].r.height/2)) 
                            && y2 > (glyphs[l].y - (glyphs[l].r.height/2))) {
                        p = RED;
                    }
                }
            }
            
            // SPI FIFO technique from Paul Stoffregen's ILI9341_t3 library:
            while(KINETISK_SPI0.SR & 0xC000); // Wait for space in FIFO
            KINETISK_SPI0.PUSHR = p | SPI_PUSHR_CTAS(1) | SPI_PUSHR_CONT;
        } // end column
    } // end scanline

    //manually end SPI transaction
    KINETISK_SPI0.SR |= SPI_SR_TCF;         // Clear transfer flag
    while((KINETISK_SPI0.SR & 0xF000) ||    // Wait for SPI FIFO to drain
             !(KINETISK_SPI0.SR & SPI_SR_TCF)); // Wait for last bit out

    digitalWrite(select_pins[eye], HIGH);          // Deselect
    TFT_SPI.endTransaction();
}
