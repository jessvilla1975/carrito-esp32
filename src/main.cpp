#include <WiFi.h>
#include <libwifi.h>
#include <libstorage.h>
#include <libprovision.h>
#include "libiot.h"
#include "libcamera.h"
#include "libmotor.h"

// ─────────────────────────────────────────────────────────────────
//  CarrIoT — ESP32-S3-CAM-N16R8
//
//  Hardware:
//    · Cámara OV2640 (integrada, pines fijos en libcamera.h)
//    · Servo izquierdo SG90 continuo → GPIO 14
//    · Servo derecho  SG90  continuo → GPIO 21
//    · Botón BOOT (GPIO 0) — mantener 3 s al encender = factory reset
//
//  AJUSTE FINO DE SERVOS:
//    Si el carrito gira solo estando "parado", edita SERVO_DEAD_BAND_US
//    en libmotor.h en pasos de 5 µs hasta que los servos queden quietos.
// ─────────────────────────────────────────────────────────────────

#ifndef FIRMWARE_VERSION
#  define FIRMWARE_VERSION "v1.0.0"
#endif

// ── ADC batería (opcional) ────────────────────────────────────────
// Divisor resistivo 100k/100k entre LiPo+ y GPIO 3.
// V_adc = V_bat/2  →  V_bat = raw * 3.3 / 4095 * 2
#define BAT_ADC_GPIO     3
#define BAT_ENABLED      false  // cambia a true si tienes el divisor
#define BAT_INTERVAL_MS  30000

static unsigned long lastBatMs = 0;

static void readAndPublishBattery() {
#if BAT_ENABLED
    int   raw     = analogRead(BAT_ADC_GPIO);
    float voltage = (float)raw * 3.3f / 4095.0f * 2.0f;
    int   level   = (int)constrain((voltage - 3.2f) / (4.2f - 3.2f) * 100.0f, 0, 100);
    publishBattery(voltage, level);
    Serial.printf("[BAT] %.2f V  %d%%\n", voltage, level);
#endif
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n========================================");
    Serial.println("  CarrIoT — ESP32-S3-CAM-N16R8");
    Serial.print("  Firmware: ");
    Serial.println(getFirmwareVersion());
    Serial.println("========================================\n");

    // ── Factory reset: mantener BOOT presionado 3 s ───────────────
    pinMode(0, INPUT_PULLUP);
    if (digitalRead(0) == LOW) {
        unsigned long t0 = millis();
        while (digitalRead(0) == LOW && (millis() - t0) < 3000) delay(10);
        if ((millis() - t0) >= 3000) {
            Serial.println("[BOOT] Factory reset...");
            factoryReset();
        }
    }

    // ── Motores: detener antes de hacer cualquier otra cosa ───────
    setupMotors();

    // ── WiFi ──────────────────────────────────────────────────────
    listWiFiNetworks();
    delay(500);
    startWiFi("carrito-cam");

    if (WiFi.status() == WL_CONNECTED) {
        setTime();
        setupIoT();

        // ── Cámara ───────────────────────────────────────────────
        if (!initCamera()) {
            Serial.println("[ERROR] Cámara no inicializada.");
            Serial.println("        Revisa el conector FPC y reinicia.");
            delay(10000);
            ESP.restart();
        }

        // Stream MJPEG en Core 1 (no bloquea el loop de MQTT en Core 0)
        startCameraStream();

        Serial.println("\n========================================");
        Serial.print("  IP del carrito: ");
        Serial.println(WiFi.localIP());
        Serial.println("  Stream: http://" + WiFi.localIP().toString() + ":81/stream");
        Serial.println("  Control MQTT:");
        Serial.println("    Topic:   " TOPIC_CMD_MOV);
        Serial.println("    Payload: {\"dir\":\"forward\",\"speed\":70}");
        Serial.println("  OTA MQTT:");
        Serial.println("    Topic:   " TOPIC_OTA_CMD);
        Serial.println("    Payload: {\"url\":\"https://...\",\"version\":\"v1.1.0\"}");
        Serial.println("========================================\n");

    } else {
        Serial.println("[WiFi] Sin conexión — Portal de configuración activo");
        Serial.println("       Conecta al AP 'CarrIoT-Setup-XXXXXX'");
        Serial.println("       Abre http://192.168.4.1 en tu navegador");
        startProvisioningAP();
    }
}

void loop() {
    if (isProvisioning()) { provisioningLoop(); return; }

    checkWiFi();
    checkMQTT();

    if (millis() - lastBatMs > BAT_INTERVAL_MS) {
        lastBatMs = millis();
        readAndPublishBattery();
    }
}
