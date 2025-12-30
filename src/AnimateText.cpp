#include <Arduino.h>

class AnimateText {
    static constexpr uint8_t MAX_FONTS = 8;

    byte buffer[8] = {};

    const byte *fonts[MAX_FONTS] = {};
    uint8_t numFonts = 0;
    uint8_t totalWidth = 0;
    uint8_t scrollMillis = 200;

    unsigned long lastScrollTime = 0;
    uint8_t scrollOffset = 0;

public:
    AnimateText &append(const byte font[]) {
        if (numFonts < MAX_FONTS) {
            fonts[numFonts++] = font;
        }
        totalWidth = numFonts * 8;
        return *this;
    }

    AnimateText &setScrollMillis(const uint8_t ms) {
        scrollMillis = ms;
        return *this;
    }

    void clear() {
        numFonts = 0;
        scrollOffset = 0;
    }


    byte *tick() {
        auto now = millis();

        if (now - lastScrollTime >= scrollMillis) {
            lastScrollTime = now;
            scrollOffset++;
            if (scrollOffset >= totalWidth) {
                scrollOffset = 0;
            }

            for (uint8_t colIdx = 0; colIdx < 8; colIdx++) {
                uint8_t virtualCol = (scrollOffset + colIdx) % totalWidth;
                uint8_t fontIdx = virtualCol / 8;
                uint8_t localCol = virtualCol % 8;

                if (localCol < 7) {
                    buffer[colIdx] = pgm_read_byte(&fonts[fontIdx][localCol]);
                } else {
                    buffer[colIdx] = 0;
                }
            }
        }
        return buffer;
    }
};
