#ifndef LIBIOT_H
#define LIBIOT_H

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <time.h>

// ── Topics MQTT del carrito ────────────────────────────────────────
#define TOPIC_CMD_MOV      "carrito/cmd/movimiento"   // suscribe
#define TOPIC_STATUS_CONN  "carrito/status/conexion"  // publica
#define TOPIC_STATUS_BAT   "carrito/status/bateria"   // publica
#define TOPIC_OTA_CMD      "carrito/ota/update"        // suscribe
#define TOPIC_OTA_STATUS   "carrito/ota/status"        // publica

// ── Variables globales ─────────────────────────────────────────────
// ssid y password son usados por libwifi.cpp
extern const char* ssid;
extern const char* password;

extern const char* mqtt_server;
extern const char* mqtt_server_ip;
extern const int   mqtt_port;
extern const char* mqtt_user;
extern const char* mqtt_password;
extern const char* root_ca;

extern WiFiClientSecure espClient;
extern PubSubClient     client;
extern time_t           now;

// ── Funciones ──────────────────────────────────────────────────────
void   setupIoT();
void   checkMQTT();
time_t setTime();
void   publishStatus();
void   publishBattery(float voltage, int levelPct);
String getMacAddress();

#endif /* LIBIOT_H */
