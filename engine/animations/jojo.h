// Jojo animation
// An example of a combined bitmap with empty portions, and dynamic pupil movement
// TODO: blinking, special expressions

void animate_frame() {
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
}
