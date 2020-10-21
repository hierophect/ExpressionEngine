// Orisa Dynamic demo
// A fully shaped based demo with random state changes and eye movement.

void animate_frame() {
    uint32_t t = micros();

    //statics
    static boolean  inMotion      = false;
    static int16_t  eyeOldX=0, eyeOldY=0, eyeOldR=0, eyeNewX=0, eyeNewY=0, eyeNewR=0;
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
            if (cs == 6 || cs == 8 || cs == 10) {
                //some states have longer stop periods
                stateDuration = random(1000000, 2000000);    //stop for set time
            } else {
                stateDuration = random(100000, 2000000);    //stop for set time
            }
            stateStartTime = t;

            glyphs[PUPIL_GLYPH].x = eyeOldX = eyeNewX;           // Save position
            glyphs[PUPIL_GLYPH].y = eyeOldY = eyeNewY;
            glyphs[PUPIL_GLYPH].c.radius = eyeOldR = eyeNewR;

            //Rectangle positions calculated by x and y components. Angles always offset by 45
            for (int i = 0; i < 4; i++) {
                glyphs[i+1].x = eyeOldX + ((st.d[i]*cos_lookup[(st.angle + (i*90)) % 359])/65535) + st.xoff[i];
                glyphs[i+1].y = eyeOldY - ((st.d[i]*sin_lookup[(st.angle + (i*90)) % 359])/65535) + st.yoff[i];
                glyphs[i+1].r.angle = ((i*90) + 45 - st.angle + 360) % 359;
            }
        } else { // Move time's not yet fully elapsed -- interpolate position
            int16_t e = ease[255 * dt / stateDuration] + 1;   // Ease curve
            uint16_t intX = eyeOldX + (((eyeNewX - eyeOldX) * e) / 256); // Interp X
            uint16_t intY = eyeOldY + (((eyeNewY - eyeOldY) * e) / 256); // and Y
            uint16_t intR = eyeOldR + (((eyeNewR - eyeOldR) * e) / 256); // and Y
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
        glyphs[PUPIL_GLYPH].x = eyeOldX;
        glyphs[PUPIL_GLYPH].y = eyeOldY;
        glyphs[PUPIL_GLYPH].c.radius = eyeOldR;

        //Rectangle positions calculated by x and y components. Angles always offset by 45
        for (int i = 0; i < 4; i++) {
            glyphs[i+1].x = eyeOldX + ((st.d[i]*cos_lookup[(st.angle + (i*90)) % 359])/65535) + st.xoff[i];
            glyphs[i+1].y = eyeOldY - ((st.d[i]*sin_lookup[(st.angle + (i*90)) % 359])/65535) + st.yoff[i];
            glyphs[i+1].r.angle = ((i*90) + 45 - st.angle + 360) % 359;
        }

        if(dt > stateDuration) {            // Time up?  Begin new move.
            int16_t  dx, dy;
            uint32_t d;
            do {                                // Pick new dest in circle
                eyeNewX = random(1024);
                eyeNewY = random(1024);
                eyeNewR = random(1024);
                dx      = (eyeNewX * 2) - 1023;
                dy      = (eyeNewY * 2) - 1023;
            } while((d = (dx * dx + dy * dy)) > (1023 * 1023)); // Keep trying

            //change the state
            //cs += 1;
            // if (cs >= statesize) {
            //     cs = 0;
            // }
            cs = random(0,10);
            stOld = st;
            st = states[cs];

            if (cs == 6 || cs == 8 || cs == 10) {
                //some states have restricted motion
                eyeNewX = map(eyeNewX, 0, 1023, 60, 70);
                eyeNewY = map(eyeNewY, 0, 1023, 60, 70);
                eyeNewR = map(eyeNewR, 0, 1023, 34, 35);
            } else {
                eyeNewX = map(eyeNewX, 0, 1023, 20, 100);
                eyeNewY = map(eyeNewY, 0, 1023, 20, 100);
                eyeNewR = map(eyeNewR, 0, 1023, 30, 40);
            }

            stateDuration  = random(144000, 300000); // ~1/14 - ~1/7 sec
            stateStartTime = t;
            inMotion    = true;            // Start move on next frame
        }
    }
}