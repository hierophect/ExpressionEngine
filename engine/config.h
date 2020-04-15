// Config stuff

//#include "graphics/defaultEye.h"        // Default eye
#include "graphics/jojo.h"        // Default eye

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   128         // Change this to 96 for 1.27" OLED.
#define DISPLAY_DC      7           // Data/command pin for ALL displays
#define DISPLAY_RESET   8           // Reset pin for ALL displays
#define DISPLAY_SEL1    9
#define DISPLAY_SEL2    10  
#define SPI_FREQ        24000000    // OLED: 24 MHz on 72 MHz Teensy only (am I overclocking?)
// #define SPI_FREQ        12000000    // OLED: 24 MHz on 72 MHz Teensy only (am I overclocking?)

/*
1111100000000000
0000011111100000
0000000000011111

0011100011100111
*/

// Color definitions
#define BLACK           0x0000
#define GREY            0x38E7
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
    //corner locations
    int16_t x1, y1, x2, y2, x3, y3, x4, y4;
} rectGlyph;

typedef struct {
    void* array;
    uint16_t width;
    uint16_t height;
} imageGlyph;

typedef struct {
    uint16_t x;
    uint16_t y;
    //bool masking;
    uint8_t type;
    union
    {
        circleGlyph c;
        rectGlyph r;
        imageGlyph i;
    };
} eyeGlyph;

eyeGlyph glyphs[] = {
    {64, 64, TYPE_CIRCLE, { .c = {15, YELLOW} }},
#if ORISA
    {32, 32, TYPE_RECT, { .r = {50,50,0, GREY} }},  //CYAN
    {32, 96, TYPE_RECT, { .r = {50,50,0, GREY} }},   //RED
    {96, 32, TYPE_RECT, { .r = {50,50,0, GREY} }},  //BLUE
    {96, 96, TYPE_RECT, { .r = {50,50,0, GREY} }}, //GREEN
#elif JOJO
    {63, 64, TYPE_IMAGE, { .i = {(void*)&image,IMAGE_WIDTH,IMAGE_HEIGHT} }},
#endif
    
    // {96, 96, true, TYPE_RECT, { .r = {34,34,0, RED} }},
    // {124, 4, TYPE_RECT, { .r = {32,255,0, BLACK} }},
    // {124, 124, TYPE_RECT, { .r = {255,32,0, BLACK} }},
    //{64, 64, TYPE_RECT, { .r = {32,128,90, RED} }},
};

typedef struct {
    uint16_t pupilX;
    uint16_t pupilY;
    uint16_t pupilR;
    int32_t duration;
    uint16_t angle; //cannot exceed 90
    uint16_t d[4];
    int16_t xoff[4]; //offsets
    int16_t yoff[4]; //offsets
} demoStates;

demoStates states[] = {  
    //{pupilX, pupilY, pupilR, duration, angle, d1, d2, d3, d4},
    {  64,     64,     40,     1000000,  0,    {80, 80, 80, 80}, {0,0,0,0}, {0,0,0,0}},        //open
    {  64,     64,     40,     1000000,  45,   {50, 50, 80, 80}, {(-20),20,0,0}, {0,0,0,0}},    //sarcastic
    {  64,     64,     30,     1000000,  45,   {50, 50, 50, 50}, {(-20),20,20,-20}, {0,0,0,0}}, //squint
    {  32,     64,     30,     500000,  45,   {50, 50, 50, 50}, {(-20),20,20,-20}, {0,0,0,0}}, //squintR
    {  96,     64,     30,     500000,  45,   {50, 50, 50, 50}, {(-20),20,20,-20}, {0,0,0,0}}, //squintL
    {  64,     64,     30,     500000,  90,    {80, 80, 80, 80}, {0,0,0,0}, {0,0,0,0}},         //open
    {  64,     64,     35,     1000000,  0,    {45, 45, 45, 45}, {10,22,10,22}, {0,12,0,-12}},  // > - <
    {  64,     64,     30,     500000,  45,    {80, 80, 80, 80}, {0,0,0,0}, {0,0,0,0}},         //open
    {  64,     64,     35,     1000000,  90,    {34, 34, 34, 34}, {0,0,0,0}, {-20,-20,0,-20}},        // ^ - ^
    {  64,     64,     30,     500000,  90,    {80, 80, 80, 80}, {0,0,0,0}, {0,0,0,0}},        //open
    {  64,     64,     20,     1000000,  0,    {45, 45, 45, 45}, {0,0,0,0}, {0,0,0,0}},         // + - +


    // {  64,     64,     30,     1000000,  45,   {10, 10, 10, 10}, {10,(-10),0,0}},
    // {  64,     64,     30,     1000000,  0,    {20, 20, 20, 20}, {0,0,0,0}},
};



