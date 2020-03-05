#include <SPI.h>
#include <Adafruit_GFX.h>

#define TFT_SPI        SPI
#define TFT_PERIPH     PERIPH_SPI

#include <Adafruit_SSD1351.h>  // OLED display library -OR-

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128 // Change this to 96 for 1.27" OLED.
#define DISPLAY_DC        7    // Data/command pin for ALL displays
#define DISPLAY_RESET     8    // Reset pin for ALL displays
#define DISPLAY_SELECT    9
#define DISPLAY_SELECT2    10  
#define SPI_FREQ 12000000  // OLED: 24 MHz on 72 MHz Teensy only

// Color definitions
#define BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF



float p = 3.1415926;
uint32_t startTime; 

Adafruit_SSD1351 disp = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &TFT_SPI,
                        DISPLAY_SELECT, DISPLAY_DC, -1);

Adafruit_SSD1351 disp2 = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &TFT_SPI,
                        DISPLAY_SELECT2, DISPLAY_DC, -1);
                        
SPISettings settings(SPI_FREQ, MSBFIRST, SPI_MODE0);

int cos_lookup[360] = {};
int sin_lookup[360] = {};

void setup() {
  Serial.begin(115200);
  //while (!Serial);
  Serial.println("Init");

  Serial.println("Reset displays");
  pinMode(DISPLAY_RESET, OUTPUT);
  digitalWrite(DISPLAY_RESET, LOW);  delay(1);
  digitalWrite(DISPLAY_RESET, HIGH); delay(50);

  //other display has been getting kinda borked by staying on
  pinMode(DISPLAY_SELECT, OUTPUT);
  digitalWrite(DISPLAY_SELECT, HIGH);  
  pinMode(DISPLAY_SELECT2, OUTPUT);
  digitalWrite(DISPLAY_SELECT2, HIGH);  

  disp.begin();
  disp.setRotation(0);
  disp.fillScreen(BLACK);
  disp2.begin();
  disp2.setRotation(0);
  disp2.fillScreen(BLACK);
  Serial.println("Init done");

  disp.setCursor(0, 5);
  disp.setTextColor(YELLOW);  
  disp.setTextSize(1);
  disp.println("Calculating Lookup");

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

  disp.fillScreen(BLACK);

  disp.setCursor(0, 5);
  disp.setTextColor(YELLOW);  
  disp.setTextSize(1);
  disp.println("Done");


  const uint8_t rotateOLED[] = { 0x74, 0x77, 0x66, 0x65 },
                mirrorOLED[] = { 0x76, 0x67, 0x64, 0x75 }; // Mirror+rotate
  // If OLED, loop through ALL eyes and set up remap register
  // from either mirrorOLED[] (first eye) or rotateOLED[] (others).
  // The OLED library doesn't normally use the remap reg (TFT does).
  disp.sendCommand(SSD1351_CMD_SETREMAP, &rotateOLED[0], 1);
  disp2.sendCommand(SSD1351_CMD_SETREMAP, &mirrorOLED[0], 1);
  delay(1000);
}

int i; 
//circle
int circleR = 0;
int circleX = 0;
int circleY = 0;
//box
int boxAngle = 0;
int boxX = 0;
int boxY = 0;
int boxWidth = 0;
int boxHeight = 0;
//box2
int boxAngle2 = 0;
int boxX2 = 0;
int boxY2 = 0;
int boxWidth2 = 0;
int boxHeight2 = 0;
//box3
int boxAngle3 = 0;
int boxX3 = 0;
int boxY3 = 0;
int boxWidth3 = 0;
int boxHeight3 = 0;

void loop() {
  uint16_t starttime = millis();

  uint8_t  screenX, screenY;
  uint16_t p = 0; 

  i++;
  int mult = 20;
  if (i>10*mult) i = 0;
  
  if (i>0 && i < (mult*1)) {
    //angery
    circleX = 64; circleY = 64;
    circleR = 64;
    boxAngle = 15;
    boxX = 64;
    boxY = 0;
    boxWidth = 160;
    boxHeight = 64;
    //box2
    boxAngle2 = 0;
    boxX2 = 0;
    boxY2 = 0;
    boxWidth2 = 0;
    boxHeight2 = 0;
    //box3
    boxAngle3 = 0;
    boxX3 = 0;
    boxY3 = 0;
    boxWidth3 = 0;
    boxHeight3 = 0;
  } else if (i < (mult*2) ) {
    //look left
    circleX = 32; circleY = 64;
    circleR = 48;
    boxAngle = 0;
    boxX = 0;
    boxY = 0;
    boxWidth = 0;
    boxHeight = 0;
        //box2
    boxAngle2 = 0;
    boxX2 = 0;
    boxY2 = 0;
    boxWidth2 = 0;
    boxHeight2 = 0;
    //box3
    boxAngle3 = 0;
    boxX3 = 0;
    boxY3 = 0;
    boxWidth3 = 0;
    boxHeight3 = 0;
  } else if (i < (mult*3) ) {
    //look up
    circleX = 64; circleY = 32;
    circleR = 48;
    boxAngle = 0;
    boxX = 0;
    boxY = 0;
    boxWidth = 0;
    boxHeight = 0;
    //box2
    boxAngle2 = 0;
    boxX2 = 0;
    boxY2 = 0;
    boxWidth2 = 0;
    boxHeight2 = 0;
    //box3
    boxAngle3 = 0;
    boxX3 = 0;
    boxY3 = 0;
    boxWidth3 = 0;
    boxHeight3 = 0;
  } else if (i < (mult*4) ) {
    //look up
    circleX = 96; circleY = 64;
    circleR = 48;
    boxAngle = 0;
    boxX = 0;
    boxY = 0;
    boxWidth = 0;
    boxHeight = 0;
    //box2
    boxAngle2 = 0;
    boxX2 = 0;
    boxY2 = 0;
    boxWidth2 = 0;
    boxHeight2 = 0;
    //box3
    boxAngle3 = 0;
    boxX3 = 0;
    boxY3 = 0;
    boxWidth3 = 0;
    boxHeight3 = 0;
  } else if (i < (mult*5) ) {
    //sarcastic
    circleX = 64; circleY = 64;
    circleR = 64;
    boxAngle = 0;
    boxX = 64;
    boxY = 32;
    boxWidth = 128;
    boxHeight = 64;
    //box2
    boxAngle2 = 0;
    boxX2 = 0;
    boxY2 = 0;
    boxWidth2 = 0;
    boxHeight2 = 0;
    //box3
    boxAngle3 = 0;
    boxX3 = 0;
    boxY3 = 0;
    boxWidth3 = 0;
    boxHeight3 = 0;
  }  else if (i < (mult*6) )  {
    //happeh
    circleX = 64; circleY = 64;
    circleR = 64;
    boxAngle = 0;
    boxX = 64;
    boxY = 96;
    boxWidth = 128;
    boxHeight = 64;
    //box2
    boxAngle2 = 0;
    boxX2 = 0;
    boxY2 = 0;
    boxWidth2 = 0;
    boxHeight2 = 0;
    //box3
    boxAngle3 = 0;
    boxX3 = 0;
    boxY3 = 0;
    boxWidth3 = 0;
    boxHeight3 = 0;
  } else if (i < (mult*7) )  {
    //who dis
    circleX = 64; circleY = 64;
    circleR = 64;
    boxAngle = 0;
    boxX = 64;
    boxY = 112;
    boxWidth = 128;
    boxHeight = 48;
    //box2
    boxAngle2 = 0;
    boxX2 = 64;
    boxY2 = 16;
    boxWidth2 = 128;
    boxHeight2 = 48;
    //box3
    boxAngle3 = 0;
    boxX3 = 0;
    boxY3 = 0;
    boxWidth3 = 0;
    boxHeight3 = 0;
  } else if (i < (mult*8) )  {
    //XD
    circleX = 64; circleY = 64;
    circleR = 64;
    boxAngle = 45;
    boxX = 96;
    boxY = 32;
    boxWidth = 128;
    boxHeight = 48;
    //box2
    boxAngle2 = 45;
    boxX2 = 96;
    boxY2 = 96;
    boxWidth2 = 48;
    boxHeight2 = 128;
    //box3
    boxAngle3 = 45;
    boxX3 = 16;
    boxY3 = 64;
    boxWidth3 = 64;
    boxHeight3 = 64;
  } else {
    //no expression
    circleX = 64; circleY = 64;
    circleR = 64;
    boxAngle = 15;
    boxX = 0;
    boxY = 0;
    boxWidth = 0;
    boxHeight = 0;
    //box2
    boxAngle2 = 0;
    boxX2 = 0;
    boxY2 = 0;
    boxWidth2 = 0;
    boxHeight2 = 0;
    //box3
    boxAngle3 = 0;
    boxX3 = 0;
    boxY3 = 0;
    boxWidth3 = 0;
    boxHeight3 = 0;
  }

  //FIRST EYEBALL-------------------------
  //manually start transaction (OLED)
  TFT_SPI.beginTransaction(settings);
  digitalWrite(DISPLAY_SELECT, LOW);
  disp.writeCommand(SSD1351_CMD_SETROW);    // Y range
  disp.spiWrite(0); disp.spiWrite(SCREEN_HEIGHT - 1);
  disp.writeCommand(SSD1351_CMD_SETCOLUMN); // X range
  disp.spiWrite(0); disp.spiWrite(SCREEN_WIDTH  - 1);
  disp.writeCommand(SSD1351_CMD_WRITERAM);  // Begin write

  //manually start data mode
  digitalWrite(DISPLAY_SELECT, LOW);                // Re-chip-select
  digitalWrite(DISPLAY_DC, HIGH);                      // Data mode

  for(screenY=0; screenY<SCREEN_HEIGHT; screenY++) {
    for(screenX=0; screenX<SCREEN_WIDTH; screenX++) {
      //default
      p = BLACK;
      
      int dsqr = (screenX - circleX)*(screenX - circleX) + (screenY - circleY)*(screenY - circleY);
      if (dsqr < (circleR*circleR)) {
        p = YELLOW;
      }
  
      //transforms for rotated rectangle 1
      int xshi = screenX - boxX;
      int yshi = screenY - boxY;
      int x2 = (cos_lookup[boxAngle]*(xshi))/65535 + (sin_lookup[boxAngle] * (yshi))/65535 + boxX;
      int y2 = (-sin_lookup[boxAngle]*(xshi))/65535 + (cos_lookup[boxAngle] * (yshi))/65535 + boxY;
      if (x2<(boxX+(boxWidth/2)) && x2>(boxX-(boxWidth/2)) && y2<(boxY+(boxHeight/2)) && y2>(boxY-(boxHeight/2))) {
        p = BLACK;
      }

      //transforms for rotated rectangle 2
      int xshi2 = screenX - boxX2;
      int yshi2 = screenY - boxY2;
      int x22 = (cos_lookup[boxAngle2]*(xshi2))/65535 + (sin_lookup[boxAngle2] * (yshi2))/65535 + boxX2;
      int y22 = (-sin_lookup[boxAngle2]*(xshi2))/65535 + (cos_lookup[boxAngle2] * (yshi2))/65535 + boxY2;
      if (x22<(boxX2+(boxWidth2/2)) && x22>(boxX2-(boxWidth2/2)) && y22<(boxY2+(boxHeight2/2)) && y22>(boxY2-(boxHeight2/2))) {
        p = BLACK;
      }

      //transforms for rotated rectangle 3
      int xshi3 = screenX - boxX3;
      int yshi3 = screenY - boxY3;
      int x23 = (cos_lookup[boxAngle3]*(xshi3))/65535 + (sin_lookup[boxAngle3] * (yshi3))/65535 + boxX3;
      int y23 = (-sin_lookup[boxAngle3]*(xshi3))/65535 + (cos_lookup[boxAngle3] * (yshi3))/65535 + boxY3;
      if (x23<(boxX3+(boxWidth3/2)) && x23>(boxX3-(boxWidth3/2)) && y23<(boxY3+(boxHeight3/2)) && y23>(boxY3-(boxHeight3/2))) {
        p = BLACK;
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

  digitalWrite(DISPLAY_SELECT, HIGH);          // Deselect
  TFT_SPI.endTransaction();


//CHEATING
if ( (i > (mult*1)) && (i < (mult*2)) ) {
    //look left
    circleX = 96; circleY = 64;
    circleR = 48;
    boxAngle = 0;
    boxX = 0;
    boxY = 0;
    boxWidth = 0;
    boxHeight = 0;
        //box2
    boxAngle2 = 0;
    boxX2 = 0;
    boxY2 = 0;
    boxWidth2 = 0;
    boxHeight2 = 0;
    //box3
    boxAngle3 = 0;
    boxX3 = 0;
    boxY3 = 0;
    boxWidth3 = 0;
    boxHeight3 = 0;
}

  //SECOND EYEBALL -------------------------
  //manually start transaction (OLED)
  TFT_SPI.beginTransaction(settings);
  digitalWrite(DISPLAY_SELECT2, LOW);
  disp2.writeCommand(SSD1351_CMD_SETROW);    // Y range
  disp2.spiWrite(0); disp2.spiWrite(SCREEN_HEIGHT - 1);
  disp2.writeCommand(SSD1351_CMD_SETCOLUMN); // X range
  disp2.spiWrite(0); disp2.spiWrite(SCREEN_WIDTH  - 1);
  disp2.writeCommand(SSD1351_CMD_WRITERAM);  // Begin write

  //manually start data mode
  digitalWrite(DISPLAY_SELECT2, LOW);                // Re-chip-select
  digitalWrite(DISPLAY_DC, HIGH);                      // Data mode

  for(screenY=0; screenY<SCREEN_HEIGHT; screenY++) {
    for(screenX=0; screenX<SCREEN_WIDTH; screenX++) {
      //default
      p = BLACK;
      
      int dsqr = (screenX - circleX)*(screenX - circleX) + (screenY - circleY)*(screenY - circleY);
      if (dsqr < (circleR*circleR)) {
        p = YELLOW;
      }
  
      //transforms for rotated rectangle 1
      int xshi = screenX - boxX;
      int yshi = screenY - boxY;
      int x2 = (cos_lookup[boxAngle]*(xshi))/65535 + (sin_lookup[boxAngle] * (yshi))/65535 + boxX;
      int y2 = (-sin_lookup[boxAngle]*(xshi))/65535 + (cos_lookup[boxAngle] * (yshi))/65535 + boxY;
      if (x2<(boxX+(boxWidth/2)) && x2>(boxX-(boxWidth/2)) && y2<(boxY+(boxHeight/2)) && y2>(boxY-(boxHeight/2))) {
        p = BLACK;
      }

      //transforms for rotated rectangle 2
      int xshi2 = screenX - boxX2;
      int yshi2 = screenY - boxY2;
      int x22 = (cos_lookup[boxAngle2]*(xshi2))/65535 + (sin_lookup[boxAngle2] * (yshi2))/65535 + boxX2;
      int y22 = (-sin_lookup[boxAngle2]*(xshi2))/65535 + (cos_lookup[boxAngle2] * (yshi2))/65535 + boxY2;
      if (x22<(boxX2+(boxWidth2/2)) && x22>(boxX2-(boxWidth2/2)) && y22<(boxY2+(boxHeight2/2)) && y22>(boxY2-(boxHeight2/2))) {
        p = BLACK;
      }

      //transforms for rotated rectangle 3
      int xshi3 = screenX - boxX3;
      int yshi3 = screenY - boxY3;
      int x23 = (cos_lookup[boxAngle3]*(xshi3))/65535 + (sin_lookup[boxAngle3] * (yshi3))/65535 + boxX3;
      int y23 = (-sin_lookup[boxAngle3]*(xshi3))/65535 + (cos_lookup[boxAngle3] * (yshi3))/65535 + boxY3;
      if (x23<(boxX3+(boxWidth3/2)) && x23>(boxX3-(boxWidth3/2)) && y23<(boxY3+(boxHeight3/2)) && y23>(boxY3-(boxHeight3/2))) {
        p = BLACK;
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

  digitalWrite(DISPLAY_SELECT2, HIGH);          // Deselect
  TFT_SPI.endTransaction();

  starttime = millis() - starttime;
  Serial.println("Frame:");
  Serial.println(starttime, DEC);

}
