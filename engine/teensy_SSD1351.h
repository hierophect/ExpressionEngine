
#define NUM_DISPLAYS (sizeof dispInfo / sizeof dispInfo[0]) // config.h pin list
Adafruit_SSD1351* displays[NUM_DISPLAYS];

void display_init() {
    // Create all display objects
    for(uint8_t i = 0; i<NUM_DISPLAYS; i++) {
        Serial.print("Create display #"); Serial.println(i);
        displays[i] = new Adafruit_SSD1351(128, 128, &TFT_SPI,
                           dispInfo[i].select, DISPLAY_DC, -1);
    }

    // Reset all displays simultaneously
    Serial.println("Reset displays");
    pinMode(DISPLAY_RESET, OUTPUT);
    digitalWrite(DISPLAY_RESET, LOW);  delay(1);
    digitalWrite(DISPLAY_RESET, HIGH); delay(50);

    // hold sel high for both screens
    pinMode(dispInfo[0].select, OUTPUT);
    digitalWrite(dispInfo[0].select, HIGH);
    pinMode(dispInfo[1].select, OUTPUT);
    digitalWrite(dispInfo[1].select, HIGH);

    // Begin all displays
    for(uint8_t i = 0; i<NUM_DISPLAYS; i++) {
        // begin the display
        Serial.print("Begin display #"); Serial.println(i);
        displays[i]->begin();
        displays[i]->setRotation(dispInfo[i].rotation);
        displays[i]->fillScreen(BLACK); //remove?
    }

    //set rotation & mirroring for first two screens manually for now
    //I'd condense this but it takes pointers, and I might want to change later.
    const uint8_t rotateOLED[] = { 0x74, 0x77, 0x66, 0x65 };
    const uint8_t mirrorOLED[] = { 0x76, 0x67, 0x64, 0x75 };
    displays[0]->sendCommand(SSD1351_CMD_SETREMAP, &mirrorOLED[0], 1);
    displays[1]->sendCommand(SSD1351_CMD_SETREMAP, &rotateOLED[0], 1);

    // TODO: implement this alternative
    // for(e=0; e<NUM_DISPLAYS; e++) {
    //   disp[e].display->sendCommand(SSD1351_CMD_SETREMAP, e ?
    //     &rotateOLED[dispInfo[e].rotation & 3] :
    //     &mirrorOLED[dispInfo[e].rotation & 3], 1);
    // }

    #ifdef ARDUINO_ARCH_SAMD
      // Set up SPI DMA on SAMD boards:
      int                dmac_id  = TFT_SPI.getDMAC_ID_TX();
      volatile uint32_t *data_reg = TFT_SPI.getDataRegister();

      Serial.println("DMA init");
      dma.allocate();
      dma.setTrigger(dmac_id);
      dma.setAction(DMA_TRIGGER_ACTON_BEAT);
    #ifdef PIXEL_DOUBLE
      // A chain of 2 linked descriptors point to the same buffer.
      // Poof, doubled scanlines.
      for(uint8_t i=0; i<2; i++) {
        descriptor[i] = dma.addDescriptor(
          NULL,               // move data
          (void *)data_reg,   // to here
          sizeof dmaBuf[0],   // this many...
          DMA_BEAT_SIZE_BYTE, // bytes/hword/words
          true,               // increment source addr?
          false);             // increment dest addr?
      }
    #else
      descriptor = dma.addDescriptor(
        NULL,               // move data
        (void *)data_reg,   // to here
        sizeof dmaBuf[0],   // this many...
        DMA_BEAT_SIZE_BYTE, // bytes/hword/words
        true,               // increment source addr?
        false);             // increment dest addr?
    #endif
      dma.setCallback(dma_callback);

    #endif // End SAMD-specific SPI DMA init
}

void display_start_SPI(uint8_t disp) {
    //manually start transaction (OLED)
    TFT_SPI.beginTransaction(settings);     // SPI transaction start
    digitalWrite(dispInfo[disp].select, LOW);    // chip select

    displays[disp]->writeCommand(SSD1351_CMD_SETROW);    // Y range
    displays[disp]->spiWrite(0);
    displays[disp]->spiWrite(SCREEN_HEIGHT - 1);
    displays[disp]->writeCommand(SSD1351_CMD_SETCOLUMN); // X range
    displays[disp]->spiWrite(0);
    displays[disp]->spiWrite(SCREEN_WIDTH  - 1);
    displays[disp]->writeCommand(SSD1351_CMD_WRITERAM);  // Begin write

    digitalWrite(dispInfo[disp].select, LOW);                // Re-chip-select
    digitalWrite(DISPLAY_DC, HIGH);                      // Data mode
}

void display_load_pixel(uint16_t p) {
    // SPI FIFO technique from Paul Stoffregen's ILI9341_t3 library:
    while(KINETISK_SPI0.SR & 0xC000); // Wait for space in FIFO
    KINETISK_SPI0.PUSHR = p | SPI_PUSHR_CTAS(1) | SPI_PUSHR_CONT;
}

void display_end_SPI(uint8_t disp) {
    //manually end SPI transaction
    KINETISK_SPI0.SR |= SPI_SR_TCF;         // Clear transfer flag
    while((KINETISK_SPI0.SR & 0xF000) ||    // Wait for SPI FIFO to drain
             !(KINETISK_SPI0.SR & SPI_SR_TCF)); // Wait for last bit out

    digitalWrite(dispInfo[disp].select, HIGH);          // Deselect
    TFT_SPI.endTransaction();
}
