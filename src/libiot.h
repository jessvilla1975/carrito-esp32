#ifndef LIBIOT_H
#define LIBIOT_H

#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Arduino.h>
#include <time.h>

// ── Topics MQTT del carrito ────────────────────────────────────────
#define TOPIC_CMD_MOV      "carrito/cmd/movimiento"   // suscribe: {"dir":"forward","speed":70}
#define TOPIC_CMD_MODE     "carrito/cmd/modo"          // suscribe: {"mode":"auto"} o {"mode":"manual"}
#define TOPIC_STATUS_CONN  "carrito/status/conexion"  // publica:  {"online":true,...}
#define TOPIC_STATUS_DIST  "carrito/status/distancia" // publica:  {"cm":45,"estado":"libre"}
#define TOPIC_STATUS_BAT   "carrito/status/bateria"   // publica:  {"voltage":"3.95","level":72}
#define TOPIC_OTA_CMD      "carrito/ota/update"        // suscribe: {"url":"...","version":"v1.1.0"}
#define TOPIC_OTA_STATUS   "carrito/ota/status"        // publica:  estado OTA

// ── Variables globales ─────────────────────────────────────────────
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

// ── Modo de operación ─────────────────────────────────────────────
typedef enum { MODE_AUTO, MODE_MANUAL } DriveMode;
extern volatile DriveMode driveMode;

// ── Funciones ──────────────────────────────────────────────────────
void   setupIoT();
void   checkMQTT();
time_t setTime();
void   publishStatus();
void   publishDistance(float cm, const char* estado);
void   publishBattery(float voltage, int levelPct);
String getMacAddress();

#endif /* LIBIOT_H */
