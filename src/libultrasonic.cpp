#include "libultrasonic.h"
#include <Arduino.h>

void setupUltrasonic() {
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIG_PIN, LOW);
    Serial.printf("[Sonar] OK — TRIG GPIO%d | ECHO GPIO%d\n", TRIG_PIN, ECHO_PIN);
}

/*
 * Dispara un pulso de 10 µs en TRIG y mide el tiempo de retorno en ECHO.
 * Velocidad del sonido ≈ 0.0343 cm/µs → distancia = tiempo / 2 * 0.0343
 * Retorna -1 si la lectura está fuera de rango.
 */
float readDistanceCm() {
    // Pulso de disparo
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Medir duración del eco (timeout = 30 ms → ~500 cm, fuera de rango útil)
    long duration = pulseIn(ECHO_PIN, HIGH, 30000);

    if (duration == 0) return -1.0f;  // sin eco = sin objeto en rango

    float distance = (duration * 0.0343f) / 2.0f;
    if (distance > MAX_DIST_CM) return -1.0f;
    return distance;
}

bool isObstacle() {
    float d = readDistanceCm();
    return (d > 0 && d < OBSTACLE_DIST_CM);
}

bool isClearPath() {
    float d = readDistanceCm();
    return (d < 0 || d >= CLEAR_DIST_CM);  // -1 = sin objeto detectado = libre
}
