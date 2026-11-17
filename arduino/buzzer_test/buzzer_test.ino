/*
 * buzzer_test.ino - hardware check for the buzzer, independent of the AUTOSAR firmware.
 * Beeps on D3, then D8, then D9 (the kit lesson uses D8/D9), 2 kHz for 0.8 s each,
 * and prints the pin on the Serial Monitor (115200). Listen which one sounds.
 */
const uint8_t kPins[] = { 3, 8, 9 };

void setup()
{
    Serial.begin(115200);
    Serial.println(F("Buzzer test: D3 -> D8 -> D9"));
}

void loop()
{
    for (uint8_t i = 0; i < sizeof(kPins); i++)
    {
        uint8_t pin = kPins[i];
        Serial.print(F("Beep on D"));
        Serial.println(pin);
        tone(pin, 2000);          /* square wave: works for passive and active buzzers */
        delay(800);
        noTone(pin);
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        delay(700);
    }
    Serial.println(F("--- pause ---"));
    delay(2000);
}
