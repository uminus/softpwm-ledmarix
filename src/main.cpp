#include <Arduino.h>
#include <avr/sleep.h>
#include <OneButton.h>
#include "Font.cpp"
#include "SoftPWM_LedMatrix.cpp"
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

PomodoroTimer timer;
SoftPWM_LedMatrix matrix(row, col);

OneButton btn;

void setup() {
    _PROTECTED_WRITE(CLKCTRL.MCLKCTRLB, 0b00000011); // div4

    matrix.setup();
    matrix.setPeriodMicros(1000);
    matrix.setDuty(.7f);

    pinMode(BUZZ_PIN, OUTPUT);
    delay(1000);

    btn.setup(BTN1_PIN, INPUT_PULLUP, true);
    btn.attachClick([] {
        switch (timer.state()) {
            case STOPPED:
                matrix.setDuty(.7f);
                timer.start();
                break;
            case RUNNING:
                matrix.setDuty(.7f);
                timer.pause();
                break;
            case PAUSED:
                matrix.setDuty(.7f);
                timer.resume();
                break;
            case COMPLETED:
                matrix.setDuty(.7f);
                timer.stop();
                break;
        }
    });

    btn.attachDoubleClick([] {
        timer.changePhase();
    });

    btn.attachLongPressStart([] {
        matrix.setDuty(0);
    });

    btn.attachLongPressStop([] {
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        noInterrupts();
        sleep_enable();
        interrupts();
        PORTC.PIN1CTRL = PORT_PULLUPEN_bm | PORT_ISC_LEVEL_gc; //pull up PC0, trigger on low level
        sleep_cpu();

        //the program will continue after waking up from here
        sleep_disable();
        PORTC.PIN1CTRL = PORT_PULLUPEN_bm; //pull up PC0, turn off the pin change interrupt
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
    0b10000000,
    0b01000000,
    0b00101010,
    0b00010100,
    0b00001000,
    0b11000100,
    0b11000010,
    0b00000001,
};

unsigned long prevDigit = 100;

static void makeTwoDigitIdx(unsigned long elapsedSecs, uint8_t out5[5]) {
    if (prevDigit == elapsedSecs) {
        return;
    }
    tone(BUZZ_PIN, 1760, 1);
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


static unsigned long nextProgress1 = 0L;
static unsigned long nextProgress2 = 0L;
static uint8_t progress2 = 0;
static bool toggle = true;
static uint8_t progress = 0b01000001;

static void updateHeader(const unsigned long elapsedSecs) {
    const auto remainingSecs = elapsedSecs % 60;
    const uint8_t n = (remainingSecs + 9) / 10; // 0..6
    uint8_t lower6 = (n == 0) ? 0u : ((1u << n) - 1u);
    if (toggle) {
        lower6 = lower6 >> 1;
    }
    header[0] = static_cast<uint8_t>(0b11000000 | lower6);
    memcpy(pixels, header, 2);
}

static void updateFooter2(const TimerState state) {
    switch (state) {
        case STOPPED:
            footer[0] = 0b11000011;
            progress2 = 0;
            break;
        case RUNNING: {
            if (nextProgress1 < millis()) {
                progress = (progress >> 1) | (progress << 7);
                footer[0] = progress;
                nextProgress1 = millis() + 125L;
                toggle = !toggle;
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

    const auto elapsedSecs = timer.tick() / 1000;
    updateHeader(elapsedSecs);
    updateFooter2(timer.state());

    const auto now = millis();
    const auto state = timer.state();
    if (state == COMPLETED || state == PAUSED) {
        if (nextProgress1 < now) {
            if (matrix.duty() > 0.1f) {
                matrix.setDuty(0.0f);
            } else {
                matrix.setDuty(.9f);
            }
            nextProgress1 = now + 300;
        }
    }
    if (state == COMPLETED) {
        // buzzer
        if (nextProgress2 < now) {
            progress2++;
            if (progress2 < 5) {
                tone(BUZZ_PIN, 1760, 80);
                nextProgress2 = now + 125;
            } else {
                progress2 = 0;
                nextProgress2 = now + 500;
            }
        }
    }

    makeTwoDigitIdx(elapsedSecs, pixels + 2);
    matrix.update(pixels);
}
