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
#define DISPLAY_NOT_SELECT    10  
#define SPI_FREQ 12000000  // OLED: 24 MHz on 72 MHz Teensy only

// Color definitions
#define  BLACK           0x0000
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
                        DISPLAY_SELECT, DISPLAY_DC, DISPLAY_RESET);

SPISettings settings(SPI_FREQ, MSBFIRST, SPI_MODE0);

int cos_lookup[360] = {};
int sin_lookup[360] = {};

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Init");

  Serial.println("Reset displays");
  pinMode(DISPLAY_RESET, OUTPUT);
  digitalWrite(DISPLAY_RESET, LOW);  delay(1);
  digitalWrite(DISPLAY_RESET, HIGH); delay(50);

  //other display has been getting kinda borked by staying on
  pinMode(DISPLAY_NOT_SELECT, OUTPUT);
  digitalWrite(DISPLAY_NOT_SELECT, HIGH);  

  disp.begin();
  disp.setRotation(0);
  Serial.println("Init done");
  disp.fillScreen(BLACK);

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
  delay(1000);
}

//int i; 
int circleR = 0;
int boxThe = 0;
float sinThe = 0;
float cosThe = 0;

void loop() {
  uint16_t starttime = millis();
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

  uint8_t  screenX, screenY;
  int box_max = 64;
  int box_min = 16;
  int box_cen = 24+16;
  uint16_t p = 0; 

  int circleX = 64;
  int circleY = 64;
  circleR++;
  if(circleR > 32) circleR = 0;
  
  boxThe++;
  if(boxThe > 359) boxThe = 0;

  for(screenY=0; screenY<SCREEN_HEIGHT; screenY++) {
    for(screenX=0; screenX<SCREEN_WIDTH; screenX++) {

      //p = BLACK;
      
//      int dsqr = (screenX - circleX)*(screenX - circleX) + (screenY - circleY)*(screenY - circleY);
//      if (dsqr < (circleR*circleR)) {
//        p = YELLOW;
//      } else {
//        p = BLACK;
//      }
  
      //transforms for rotated rectangle
      int xshi = screenX - box_cen;
      int yshi = screenY - box_cen;
      int x2 = (cos_lookup[boxThe]*(xshi))/65535 + (sin_lookup[boxThe] * (yshi))/65535 + box_cen;
      int y2 = (-sin_lookup[boxThe]*(xshi))/65535 + (cos_lookup[boxThe] * (yshi))/65535 + box_cen;

      if (x2<box_max && x2>box_min && y2<box_max && y2>box_min) {
        p = BLUE;
      } else {
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

  starttime = millis() - starttime;
  Serial.println("Frame:");
  Serial.println(starttime, DEC);

}
