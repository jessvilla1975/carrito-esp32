#include "libmotor.h"
#include <Arduino.h>
#include <string.h>

/*
 * Convierte microsegundos → duty cycle de 14 bits para 50 Hz.
 * Período = 20 000 µs → 16 384 counts
 * duty = us × 16 384 / 20 000
 */
static inline uint32_t usToDuty(uint16_t us) {
    return (uint32_t)((uint64_t)us * 16384ULL / 20000ULL);
}

static void servoWrite(uint8_t channel, uint16_t us) {
    us = (uint16_t)constrain(us, SERVO_MIN_US, SERVO_MAX_US);
    ledcWrite(channel, usToDuty(us));
}

void setupMotors() {
    ledcSetup(SERVO_LEFT_CHAN,  SERVO_FREQ_HZ, SERVO_RESOLUTION);
    ledcSetup(SERVO_RIGHT_CHAN, SERVO_FREQ_HZ, SERVO_RESOLUTION);
    ledcAttachPin(SERVO_LEFT_GPIO,  SERVO_LEFT_CHAN);
    ledcAttachPin(SERVO_RIGHT_GPIO, SERVO_RIGHT_CHAN);
    stopMotors();
    Serial.println("[Motor] OK — GPIO " + String(SERVO_LEFT_GPIO) +
                   " (izq) | GPIO " + String(SERVO_RIGHT_GPIO) + " (der)");
}

/*
 * Comando MQTT esperado: {"dir":"forward","speed":70}
 * dir: "forward" | "backward" | "left" | "right" | "stop"
 * speed: 0-100
 *
 * Tracción diferencial (servo derecho montado espejado):
 *   FORWARD  → izq >1500  der <1500
 *   BACKWARD → izq <1500  der >1500
 *   LEFT     → izq <1500  der <1500  (giro sobre el eje)
 *   RIGHT    → izq >1500  der >1500  (giro sobre el eje)
 */
void handleMovement(const char* dir, int speed) {
    speed = constrain(speed, 0, 100);
    int delta = speed * 5;  // 0-500 µs desde el centro

    uint16_t leftUs  = SERVO_STOP_US + SERVO_DEAD_BAND_US;
    uint16_t rightUs = SERVO_STOP_US - SERVO_DEAD_BAND_US;

    if (strcmp(dir, "forward") == 0) {
        leftUs  = SERVO_STOP_US + delta;
        rightUs = SERVO_STOP_US - delta;
    } else if (strcmp(dir, "backward") == 0) {
        leftUs  = SERVO_STOP_US - delta;
        rightUs = SERVO_STOP_US + delta;
    } else if (strcmp(dir, "left") == 0) {
        leftUs  = SERVO_STOP_US - delta;
        rightUs = SERVO_STOP_US - delta;
    } else if (strcmp(dir, "right") == 0) {
        leftUs  = SERVO_STOP_US + delta;
        rightUs = SERVO_STOP_US + delta;
    } else {
        stopMotors();
        return;
    }

    servoWrite(SERVO_LEFT_CHAN,  leftUs);
    servoWrite(SERVO_RIGHT_CHAN, rightUs);
    Serial.printf("[Motor] dir=%s spd=%d  izq=%dµs der=%dµs\n", dir, speed, leftUs, rightUs);
}

void stopMotors() {
    ledcWrite(SERVO_LEFT_CHAN,  usToDuty(SERVO_STOP_US + SERVO_DEAD_BAND_US));
    ledcWrite(SERVO_RIGHT_CHAN, usToDuty(SERVO_STOP_US - SERVO_DEAD_BAND_US));
}
