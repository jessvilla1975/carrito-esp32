# CarrIoT — ESP32-S3-CAM-N16R8

Carrito con cámara OV2640, control diferencial mediante 2x servos SG90,
comunicación MQTT/TLS con broker EMQX en AWS EC2, y OTA desde AWS S3.

## Hardware

| Componente | Detalle |
|---|---|
| Microcontrolador | ESP32-S3-CAM-N16R8 (16 MB Flash QIO, 8 MB PSRAM OPI) |
| Cámara | OV2640 integrada |
| Servo izquierdo | SG90 rotación continua → GPIO 14 |
| Servo derecho | SG90 rotación continua → GPIO 21 |
| Botón factory reset | BOOT (GPIO 0) — mantener 3 s al encender |

## Características del firmware

- **Stream MJPEG** en `http://<IP>:81/stream` (Core 1, FreeRTOS)
- **Control por MQTT/TLS** (broker EMQX en AWS EC2)
- **OTA** vía HTTPS desde AWS S3
- **Provisioning WiFi** mediante portal cautivo (`CarrIoT-Setup-XXXXXX`)
- **Credenciales WiFi** guardadas en NVS (no hardcodeadas)

## Topics MQTT

| Topic | Dirección | Payload ejemplo |
|---|---|---|
| `carrito/cmd/movimiento` | ESP recibe | `{"dir":"forward","speed":70}` |
| `carrito/status/conexion` | ESP publica | `{"online":true,"ip":"...","fw":"v1.0.0"}` |
| `carrito/status/bateria` | ESP publica | `{"voltage":"3.95","level":72}` |
| `carrito/ota/update` | ESP recibe | `{"url":"https://...","version":"v1.1.0"}` |
| `carrito/ota/status` | ESP publica | estado de la actualización |

Valores de `dir`: `forward`, `backward`, `left`, `right`, `stop`

## Configuración

1. Copia `.env.template` a `.env` y completa los valores:
   ```
   WIFI_SSID=MI_RED
   WIFI_PASSWORD=mi_clave
   MQTT_SERVER=mqtt.tudominio.com
   MQTT_PORT=8883
   MQTT_USER=carrito
   MQTT_PASSWORD=clave_mqtt
   ```

2. Compila y sube:
   ```bash
   pio run -e cam --target upload
   pio device monitor
   ```

## GitHub Actions / CI-CD

El pipeline `.github/workflows/firmware.yml`:
- **Cada push a `main`**: compila el firmware con PlatformIO.
- **Cada tag `vX.Y.Z`**: compila + sube el `.bin` a S3 + publica notificación OTA por MQTT.

### Secrets necesarios en GitHub

| Secret | Descripción |
|---|---|
| `WIFI_SSID` / `WIFI_PASSWORD` | Red WiFi del entorno de prueba |
| `MQTT_SERVER` / `MQTT_PORT` | Broker EMQX |
| `MQTT_USER` / `MQTT_PASSWORD` | Credenciales MQTT |
| `AWS_ACCESS_KEY_ID` / `AWS_SECRET_ACCESS_KEY` | IAM para S3 |
| `AWS_REGION` | Región AWS (ej. `us-east-1`) |
| `S3_BUCKET_NAME` | Nombre del bucket |

## Ajuste fino de servos

Si el carrito gira solo estando en "stop", edita `SERVO_DEAD_BAND_US`
en `src/libmotor.h` en pasos de 5 µs hasta que los servos queden quietos.

## Estructura del proyecto

```
carrito-esp32/
├── .github/workflows/firmware.yml  # CI/CD
├── scripts/add_env_defines.py      # Inyección de credenciales en build
├── src/
│   ├── main.cpp          # Punto de entrada
│   ├── libiot.h/cpp      # MQTT / TLS / credenciales
│   ├── libcamera.h/cpp   # OV2640 + stream MJPEG
│   ├── libmotor.h/cpp    # Servos SG90
│   ├── libwifi.h/cpp     # Conexión WiFi
│   ├── libstorage.h/cpp  # NVS (credenciales persistentes)
│   ├── libprovision.h/cpp# Portal cautivo de configuración
│   └── libota.h/cpp      # Actualización OTA
├── partitions.csv         # Tabla de particiones 16 MB (OTA)
├── platformio.ini
├── .env.template
└── .gitignore
```
