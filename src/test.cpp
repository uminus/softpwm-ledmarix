#include <Arduino.h>


constexpr uint8_t pin[] = {
    // PIN_PA1, PIN_PA2, PIN_PA3, PIN_PA4, PIN_PA5, PIN_PA6, PIN_PA7,
    // PIN_PB0, PIN_PB1,PIN_PB2,PIN_PB3,PIN_PB4,PIN_PB5,PIN_PB6,PIN_PB7,
    PIN_PC0,PIN_PC1,PIN_PC2,PIN_PC3,PIN_PC4,PIN_PC5,
};
void setup() {
    for (unsigned char thisPin : pin) {
        pinMode(thisPin, OUTPUT);
    }
}

void loop() {
    for (unsigned char thisPin : pin) {
        digitalWrite(thisPin, HIGH);
    }
    delay(1);
    for (unsigned char thisPin : pin) {
        digitalWrite(thisPin, LOW);
    }
    delay(1);
}
