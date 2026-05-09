#include <WiFi.h>
#include <libwifi.h>
#include <libstorage.h>
#include <libprovision.h>
#include "libiot.h"
#include "libmotor.h"
#include "libultrasonic.h"

// ─────────────────────────────────────────────────────────────────
//  CarrIoT — ESP32-S3-DevKitC-1
//
//  Hardware:
//    · Servo izquierdo SG90 continuo → GPIO 20
//    · Servo derecho  SG90  continuo → GPIO 21
//    · HC-SR04 TRIG → GPIO 38 | ECHO → GPIO 39
//    · Botón BOOT (GPIO 0) — mantener 3 s = factory reset
//
//  Modos de operación:
//    AUTO   (default): evasión autónoma de obstáculos
//    MANUAL: controlado por MQTT (topic carrito/cmd/movimiento)
//
//  Cambio de modo via MQTT:
//    Topic:   carrito/cmd/modo
//    Payload: {"mode":"manual"}  o  {"mode":"auto"}
// ─────────────────────────────────────────────────────────────────

#ifndef FIRMWARE_VERSION
#  define FIRMWARE_VERSION "v1.0.0"
#endif

// ── Parámetros del comportamiento autónomo ────────────────────────
#define AUTO_SPEED          65    // velocidad crucero (0-100)
#define TURN_SPEED          55    // velocidad de giro al evadir
#define DIST_PUBLISH_MS   1000    // publicar distancia cada 1 s
#define BAT_INTERVAL_MS  30000    // publicar batería cada 30 s

// ── ADC batería (opcional) ────────────────────────────────────────
#define BAT_ADC_GPIO     3
#define BAT_ENABLED      false

// ── Estado interno del modo autónomo ─────────────────────────────
typedef enum {
    AUTO_FORWARD,       // avanzando
    AUTO_STOPPING,      // frenando antes de girar
    AUTO_TURNING,       // girando para buscar camino libre
} AutoState;

static AutoState    autoState      = AUTO_FORWARD;
static unsigned long turnStartMs   = 0;
static int           turnDir       = 1;    // 1=derecha, -1=izquierda (alterna)
static unsigned long lastDistMs    = 0;
static unsigned long lastBatMs     = 0;

// ── Evasión autónoma ─────────────────────────────────────────────
static void autonomousLoop() {
    float dist = readDistanceCm();
    const char* estadoStr = (dist < 0) ? "fuera_rango" :
                            (dist < OBSTACLE_DIST_CM) ? "obstaculo" :
                            (dist < CLEAR_DIST_CM)    ? "precaucion" : "libre";

    // Publicar distancia periódicamente
    if (millis() - lastDistMs > DIST_PUBLISH_MS) {
        lastDistMs = millis();
        if (dist > 0) {
            publishDistance(dist, estadoStr);
            Serial.printf("[Sonar] %.1f cm — %s\n", dist, estadoStr);
        }
    }

    switch (autoState) {

        case AUTO_FORWARD:
            if (dist > 0 && dist < OBSTACLE_DIST_CM) {
                // Obstáculo detectado → frenar
                stopMotors();
                Serial.printf("[Auto] Obstáculo a %.1f cm — frenando\n", dist);
                autoState = AUTO_STOPPING;
                turnStartMs = millis();
            } else {
                handleMovement("forward", AUTO_SPEED);
            }
            break;

        case AUTO_STOPPING:
            // Pequeña pausa antes de girar (300 ms)
            if (millis() - turnStartMs > 300) {
                // Elegir dirección de giro alternando
                const char* dir = (turnDir > 0) ? "right" : "left";
                handleMovement(dir, TURN_SPEED);
                Serial.printf("[Auto] Girando a la %s...\n",
                              (turnDir > 0) ? "derecha" : "izquierda");
                autoState   = AUTO_TURNING;
                turnStartMs = millis();
            }
            break;

        case AUTO_TURNING:
            if (isClearPath()) {
                // Camino libre encontrado → reanudar avance
                Serial.println("[Auto] Camino libre — reanudando");
                turnDir = -turnDir;  // alternar dirección en el próximo giro
                autoState = AUTO_FORWARD;
            } else if (millis() - turnStartMs > 2000) {
                // Giró 2 s sin encontrar camino → intentar la otra dirección
                Serial.println("[Auto] Sin camino — probando otro lado");
                turnDir = -turnDir;
                const char* dir = (turnDir > 0) ? "right" : "left";
                handleMovement(dir, TURN_SPEED);
                turnStartMs = millis();
            }
            break;
    }
}

// ── Batería ───────────────────────────────────────────────────────
static void readAndPublishBattery() {
#if BAT_ENABLED
    int   raw     = analogRead(BAT_ADC_GPIO);
    float voltage = (float)raw * 3.3f / 4095.0f * 2.0f;
    int   level   = (int)constrain((voltage - 3.2f) / (4.2f - 3.2f) * 100.0f, 0, 100);
    publishBattery(voltage, level);
    Serial.printf("[BAT] %.2f V  %d%%\n", voltage, level);
#endif
}

// ─────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n========================================");
    Serial.println("  CarrIoT — ESP32-S3-DevKitC-1");
    Serial.println("  Modo: Evasión autónoma de obstáculos");
    Serial.print  ("  Firmware: ");
    Serial.println(getFirmwareVersion());
    Serial.println("========================================\n");

    // ── Factory reset: mantener BOOT 3 s ─────────────────────────
    pinMode(0, INPUT_PULLUP);
    if (digitalRead(0) == LOW) {
        unsigned long t0 = millis();
        while (digitalRead(0) == LOW && (millis() - t0) < 3000) delay(10);
        if ((millis() - t0) >= 3000) { factoryReset(); }
    }

    // ── Inicializar periféricos ───────────────────────────────────
    setupMotors();
    setupUltrasonic();

    // ── WiFi ──────────────────────────────────────────────────────
    listWiFiNetworks();
    delay(500);
    startWiFi("carrito");

    if (WiFi.status() == WL_CONNECTED) {
        setTime();
        setupIoT();

        Serial.println("\n========================================");
        Serial.print  ("  IP: "); Serial.println(WiFi.localIP());
        Serial.println("  MQTT: carbot.ddnsfree.com:8883");
        Serial.println("  Topics:");
        Serial.println("    " TOPIC_CMD_MODE "  {\"mode\":\"manual\"}");
        Serial.println("    " TOPIC_CMD_MOV  "  {\"dir\":\"forward\",\"speed\":70}");
        Serial.println("    " TOPIC_STATUS_DIST " (distancia publicada c/1s)");
        Serial.println("  Modo inicial: AUTÓNOMO");
        Serial.println("========================================\n");
    } else {
        Serial.println("[WiFi] Sin conexión — portal de configuración activo");
        startProvisioningAP();
    }
}

void loop() {
    if (isProvisioning()) { provisioningLoop(); return; }

    checkWiFi();
    checkMQTT();

    // ── Comportamiento según el modo activo ───────────────────────
    if (driveMode == MODE_AUTO) {
        autonomousLoop();
    }
    // En MODE_MANUAL los comandos llegan por MQTT (callback en libiot.cpp)

    // ── Batería periódica ─────────────────────────────────────────
    if (millis() - lastBatMs > BAT_INTERVAL_MS) {
        lastBatMs = millis();
        readAndPublishBattery();
    }
}
