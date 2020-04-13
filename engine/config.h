// Config stuff

//#include "graphics/defaultEye.h"        // Default eye
#include "graphics/anime.h"        // Default eye

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
#define TYPE_IMAGE      2

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
    void* array;
    uint16_t width;
    uint16_t height;
} imageGlyph;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t type;
    union
    {
        circleGlyph c;
        rectGlyph r;
        imageGlyph i;
    };
} eyeGlyph;

eyeGlyph glyphs[] = {  
    {64, 64, TYPE_CIRCLE, { .c = {30, YELLOW} }},
    {63, 64, TYPE_IMAGE, { .i = {(void*)&image,IMAGE_WIDTH,IMAGE_HEIGHT} }},
    //{4, 124, TYPE_RECT, { .r = {32,255,0, BLACK} }},
    // {124, 4, TYPE_RECT, { .r = {32,255,0, BLACK} }},
    // {124, 124, TYPE_RECT, { .r = {255,32,0, BLACK} }},
    //{64, 64, TYPE_RECT, { .r = {32,128,90, RED} }},
};