
void animate_frame() {
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
}