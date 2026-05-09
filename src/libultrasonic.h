#ifndef LIBULTRASONIC_H
#define LIBULTRASONIC_H

/*
 * Sensor ultrasónico HC-SR04
 *
 * Conexión:
 *   VCC  → 5V
 *   GND  → GND
 *   TRIG → GPIO 38  (salida: pulso de 10 µs)
 *   ECHO → GPIO 39  (entrada: pulso proporcional a la distancia)
 *
 * Rango útil: 2 cm – 400 cm
 * Precisión:  ±3 mm
 */

#define TRIG_PIN          38
#define ECHO_PIN          39

#define OBSTACLE_DIST_CM  25   // distancia a la que se considera obstáculo
#define CLEAR_DIST_CM     40   // distancia mínima para considerar el camino libre
#define MAX_DIST_CM       300  // distancia máxima válida (evita lecturas falsas)

/** Umbral en modo manual: si la lectura es válida y ≤ este valor → buzzer de alerta */
#define MANUAL_BUZZER_ALERT_CM  10

void    setupUltrasonic();
float   readDistanceCm();
bool    isObstacle();
bool    isClearPath();

#endif /* LIBULTRASONIC_H */
