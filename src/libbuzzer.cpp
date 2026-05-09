#include "libbuzzer.h"

static bool          s_alert   = false;
static unsigned long s_phaseT0 = 0;
static bool          s_on      = false;

void setupBuzzer() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    s_alert   = false;
    s_on      = false;
}

void buzzerSetAlert(bool on) {
    if (on == s_alert) return;
    s_alert = on;
    if (!s_alert) {
        digitalWrite(BUZZER_PIN, LOW);
        s_on = false;
        return;
    }
    // Inicio del patrón intermitente (menos agresivo que tono continuo)
    s_phaseT0 = millis();
    s_on      = true;
    digitalWrite(BUZZER_PIN, HIGH);
}

void buzzerLoop() {
    if (!s_alert) return;

    const unsigned long ON_MS  = 140;
    const unsigned long OFF_MS = 120;
    const unsigned long now    = millis();

    if (s_on) {
        if (now - s_phaseT0 >= ON_MS) {
            s_on      = false;
            s_phaseT0 = now;
            digitalWrite(BUZZER_PIN, LOW);
        }
    } else {
        if (now - s_phaseT0 >= OFF_MS) {
            s_on      = true;
            s_phaseT0 = now;
            digitalWrite(BUZZER_PIN, HIGH);
        }
    }
}
