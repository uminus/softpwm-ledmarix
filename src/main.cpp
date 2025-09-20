#include <Arduino.h>
#include "Font.cpp"
#include "SoftPWM_LedMatrix.cpp"

constexpr uint8_t row[8] = {
    PIN_PA5, PIN_PA6, PIN_PA7, PIN_PB5, PIN_PB4, PIN_PB3, PIN_PB2, PIN_PB1,
};

constexpr uint8_t col[8] = {
    PIN_PB0, PIN_PC0, PIN_PC1, PIN_PC2, PIN_PC3, PIN_PA1, PIN_PA2, PIN_PA3,
};

SoftPWM_LedMatrix matrix(row, col);

void setup() {
    matrix.setup();
}

static uint8_t footer[] = {
    0b10000000,
};

byte pixels[8] = {
    0b10101010,
    0b01010101,
    0b00101010,
    0b00010100,
    0b00001000,
    0b11000100,
    0b11000010,
    0b00000001,
};

uint8_t prevDigit = 100;
static void makeTwoDigitIdx(uint8_t digit, uint8_t out5[5]) {
    if (prevDigit == digit) {
        return;
    }
    prevDigit = digit;

    uint8_t leftDigit = digit / 10;
    uint8_t rightDigit = digit % 10;
    for (int i = 0; i < 5; ++i) {
        uint8_t l;
        if (leftDigit == 0) {
            l = pgm_read_byte(&FONT_NONE) & 0b111;
        } else {
            l = pgm_read_byte(&FONT3x5[leftDigit][i]) & 0b111;
        }

        uint8_t r = pgm_read_byte(&FONT3x5[rightDigit][i]) & 0b111;
        out5[i] = static_cast<uint8_t>((l << 5) | r);
    }
}

static void updateFooter() {
    footer[0] = (footer[0] >> 1) | (footer[0] << 7);
    memcpy(pixels + 7, footer, 1);
}

static uint8_t counter = 0;
static bool increasing = true;
static unsigned long next = 0L;
static unsigned long nextCountUpMs = 0L;

void loop() {
    const auto now = micros();

    if (next < now) {
        const auto currentDuty = matrix.duty();
        if (increasing) {
            const auto nextDuty = matrix.setDuty(currentDuty + 0.01f);
            if (nextDuty >= 1.0f) {
                increasing = false;
            }
        } else {
            const auto nextDuty = matrix.setDuty(currentDuty - 0.01f);
            if (nextDuty <= 0.1f) {
                increasing = true;
            }
        }
        next = now + 10000;
    }

    if (now > nextCountUpMs) {
        counter++;
        nextCountUpMs = now + 100000;
        updateFooter();
    }

    if (counter > 99) {
        counter = 0;
    }

    makeTwoDigitIdx(counter, pixels + 2);
    matrix.update(pixels);
}
