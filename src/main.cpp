#include <Arduino.h>
#include <avr/sleep.h>
#include <OneButton.h>
#include "Font.cpp"
#include "SoftPWM_LedMatrix.cpp"
#include "AnimateText.cpp"
#include "PomodoroTimer.cpp"

constexpr uint8_t row[8] = {
    PIN_PB3, PIN_PA6, PIN_PB1, PIN_PB7, PIN_PA3, PIN_PB0, PIN_PC5, PIN_PC2,
};

constexpr uint8_t col[8] = {
    PIN_PA7, PIN_PC4, PIN_PC3, PIN_PB4, PIN_PC0, PIN_PB5, PIN_PA5, PIN_PA4,
};

#define BUZZ_PIN PIN_PB2
#define BTN1_PIN PIN_PC1
#define BTN2_PIN PIN_PB6

AnimateText text;
PomodoroTimer timer;
SoftPWM_LedMatrix matrix(row, col);

OneButton btn;

void setup() {
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0b00000011); // div4

    matrix.setup();
    matrix.setPeriodMicros(1000);
    matrix.setDuty(.7f);

    pinMode(BUZZ_PIN, OUTPUT);

    text
            .append(TSUKI)
            .append(KIWAMI)
            .append(TEI)
            .append(SO)
            .setScrollMillis(100);

    delay(1000);
}

byte pixels[8];


void loop() {
    matrix.update(text.tick());
}
