#ifndef LIBBUZZER_H
#define LIBBUZZER_H

#include <Arduino.h>

/*
 * Buzzer de alerta (módulo activo 3.3 V típico)
 *
 *   + del buzzer → GPIO BUZZER_PIN (p. ej. vía transistor si el consumo es alto)
 *   − → GND
 */

#define BUZZER_PIN 4

void setupBuzzer();
void buzzerSetAlert(bool on);
void buzzerLoop();

#endif
