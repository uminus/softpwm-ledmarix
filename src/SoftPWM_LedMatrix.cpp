#include <Arduino.h>

/**
 * A software PWM–controlled LED matrix driver.
 */
class SoftPWM_LedMatrix {
    const uint8_t *pinRows_;
    const uint8_t *pinCols_;

    uint16_t periodMicros_ = 2000;
    float duty_ = 0.9f;

    unsigned long nextStepMicros_ = 0;
    bool isNextHigh = true;
    uint8_t currentCol = 0;

public:
    explicit SoftPWM_LedMatrix(const uint8_t pinRows[8], const uint8_t pinCols[8])
        : pinRows_(pinRows), pinCols_{pinCols} {
    }

    /**
     * @param duty 0.0f to 1.0f (default: 0.9f)
     */
    float setDuty(const float duty) {
        if (duty <= 0.0f) {
            duty_ = 0.0f;
        } else if (duty >= 1.0f) {
            duty_ = 1.0f;
        } else {
            duty_ = duty;
        }
        return duty_;
    }

    [[nodiscard]] float duty() const {
        return duty_;
    }

    /**
     * @param periodMicros (default: 2000 [500Hz])
     */
    void setPeriodMicros(const uint16_t periodMicros) {
        periodMicros_ = periodMicros;
    }

    void setup() const {
        for (int thisPin = 0; thisPin < 8; thisPin++) {
            pinMode(pinCols_[thisPin], OUTPUT);
            digitalWrite(pinCols_[thisPin], LOW);

            pinMode(pinRows_[thisPin], OUTPUT);
            digitalWrite(pinRows_[thisPin], HIGH);
        }
    }

    void update(const byte pixels[8]) {
        const auto now = micros();
        if (nextStepMicros_ > now) {
            return;
        }
        const int durationHigh = static_cast<int>(periodMicros_ * duty_);
        if (isNextHigh) {
            nextStepMicros_ = now + durationHigh;
            isNextHigh = false;
        } else {
            nextStepMicros_ = now + periodMicros_ - durationHigh;
            isNextHigh = true;
            currentCol = currentCol == 7 ? 0 : currentCol + 1;
        }

        for (uint8_t thisCol = 0; thisCol < 8; thisCol++) {
            digitalWrite(pinCols_[thisCol], thisCol == currentCol ? HIGH : LOW);
        }

        const auto rowByte = pixels[currentCol];
        for (uint8_t thisRow = 0; thisRow < 8; ++thisRow) {
            const bool bit = (rowByte >> thisRow & 0x01) != 0;
            const int colVal = bit && isNextHigh ? LOW : HIGH;
            digitalWrite(pinRows_[thisRow], colVal);
        }
    }
};
