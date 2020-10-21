void animate_frame() {
    // Set rotation of test rectangle
    if (glyphs[0].r.angle < 359) {
        glyphs[0].r.angle += 2;
        glyphs[1].r.angle += 2;
        glyphs[2].r.angle += 2;
        glyphs[3].r.angle += 2;
    } else {
        glyphs[0].r.angle = 0;
        glyphs[1].r.angle = 0;
        glyphs[2].r.angle = 0;
        glyphs[3].r.angle = 0;
    }
}