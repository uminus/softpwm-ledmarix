#include <Arduino.h>
#include <avr/sleep.h>
#include <OneButton.h>
#include "Font.cpp"
#include "SoftPWM_LedMatrix.cpp"
#include "PomodoroTimer.cpp"

constexpr uint8_t row[8] = {
    PIN_PA5, PIN_PA6, PIN_PA7, PIN_PB5, PIN_PB4, PIN_PB3, PIN_PB2, PIN_PB1,
};

constexpr uint8_t col[8] = {
    PIN_PB0, PIN_PC0, PIN_PC1, PIN_PC2, PIN_PC3, PIN_PA1, PIN_PA2, PIN_PA3,
};

PomodoroTimer timer;
SoftPWM_LedMatrix matrix(row, col);

OneButton btn;

void setup() {
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0b00000011); // div4
    delay(1000);

    timer.setDurationMillis(120000L);

    matrix.setup();
    matrix.setPeriodMicros(2000);
    matrix.setDuty(.7f);

    btn.setup(PIN_PA4, INPUT_PULLUP, true);
    btn.attachClick([] {
        switch (timer.state()) {
            case STOPPED:
                matrix.setDuty(.7f);
                timer.start();
                break;
            case RUNNING:
                timer.pause();
                break;
            case PAUSED:
                timer.resume();
                break;
            case COMPLETED:
                matrix.setDuty(.7f);
                timer.stop();
                break;
        }
    });

    btn.attachLongPressStart([] {
        matrix.setDuty(0);
    });

    btn.attachLongPressStop([] {
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        noInterrupts();
        sleep_enable();
        interrupts();
        PORTA.PIN4CTRL = PORT_PULLUPEN_bm | PORT_ISC_LEVEL_gc; //pull up PA4, trigger on low level
        sleep_cpu();

        //the program will continue after waking up from here
        sleep_disable();
        PORTA.PIN4CTRL = PORT_PULLUPEN_bm; //pull up PA4, turn off the pin change interrupt
    });
}

static uint8_t header[2] = {
    0b11111111,
    0b00000000,
};

static uint8_t footer[1] = {
    0b10000000,
};

byte pixels[8] = {
    0b00000000,
    0b00000000,
    0b00101010,
    0b00010100,
    0b00001000,
    0b11000100,
    0b11000010,
    0b00000000,
};

uint8_t prevDigit = 100;

static void makeTwoDigitIdx(uint8_t elapsedSecs, uint8_t out5[5]) {
    if (prevDigit == elapsedSecs) {
        return;
    }
    prevDigit = elapsedSecs;

    uint8_t leftDigit;
    uint8_t rightDigit;

    if (elapsedSecs < 99) {
        // seconds
        leftDigit = elapsedSecs / 10 % 10;
        rightDigit = elapsedSecs % 10;
    } else {
        // minutes
        uint8_t elapsedMins = elapsedSecs / 60;
        leftDigit = elapsedMins / 10 % 10;
        rightDigit = elapsedMins % 10;
    }
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


static unsigned long nextProgress = 0L;
static uint8_t progress = 0b01000001;

static void updateHeader(const unsigned long elapsedSecs) {
    if (prevDigit == elapsedSecs) {
        return;
    }
    const auto remainingSecs = elapsedSecs % 60;
    const uint8_t n = remainingSecs / 10;
    const uint8_t lower5 = (n == 0) ? 0u : ((1u << n) - 1u);
    header[0] = static_cast<uint8_t>(0b11000000 | lower5);
    memcpy(pixels, header, 2);
}

static void updateFooter2(const TimerState state) {
    switch (state) {
        case STOPPED:
            footer[0] = 0b11000011;
            break;
        case RUNNING: {
            if (nextProgress < millis()) {
                progress = (progress >> 1) | (progress << 7);
                footer[0] = progress;
                nextProgress = millis() + 125L;
            }
            break;
        }
        case PAUSED:
            progress = 0b01000001;
            footer[0] = 0b00000011;
            break;
        case COMPLETED:
            footer[0] = 0b11111111;
            break;
        default:
            footer[0] = 0b00000000;
            break;
    }
    memcpy(pixels + 7, footer, 1);
}

void loop() {
    btn.tick();

    const auto elapsedSecs = timer.tick() / 1000L;
    updateHeader(elapsedSecs);
    updateFooter2(timer.state());

    if (timer.state() == COMPLETED) {
        if (nextProgress < millis()) {
            if (matrix.duty() > 0.1f) {
                matrix.setDuty(0.0f);
            } else {
                matrix.setDuty(.9f);
            }
            nextProgress = millis() + 300L;
        }
    }

    makeTwoDigitIdx(elapsedSecs, pixels + 2);
    matrix.update(pixels);
}
