# CarrIoT — ESP32-S3-DevKitC-1

Carrito autónomo con evasión de obstáculos por sensor ultrasónico, control
diferencial mediante 2× servos SG90, comunicación MQTT/TLS con broker EMQX en
AWS EC2, OTA desde AWS S3 y dashboard web en GitHub Pages.

## Hardware

| Componente | Detalle |
|---|---|
| Microcontrolador | ESP32-S3-DevKitC-1 (8 MB Flash QIO) |
| Sensor de distancia | HC-SR04 — TRIG GPIO 38 / ECHO GPIO 39 |
| Servo izquierdo | SG90 rotación continua → GPIO 20 |
| Servo derecho | SG90 rotación continua → GPIO 21 |
| Botón factory reset | BOOT (GPIO 0) — mantener 3 s al encender |

## Modos de operación

| Modo | Descripción |
|---|---|
| **Autónomo** (por defecto) | El carrito avanza y detecta obstáculos con el HC-SR04. Cuando detecta algo a menos de 25 cm, se detiene, gira hasta encontrar un camino libre y reanuda la marcha. |
| **Manual** | Recibe comandos de dirección por MQTT desde el dashboard o cualquier cliente. |

El modo se cambia publicando en `carrito/cmd/modo`.

## Topics MQTT

| Topic | Dirección | Payload ejemplo |
|---|---|---|
| `carrito/cmd/modo` | ESP recibe | `{"mode":"auto"}` / `{"mode":"manual"}` |
| `carrito/cmd/movimiento` | ESP recibe (solo en manual) | `{"dir":"forward","speed":70}` |
| `carrito/status/distancia` | ESP publica | `{"cm":34.5,"estado":"libre"}` |
| `carrito/status/conexion` | ESP publica | `{"online":true,"ip":"...","fw":"v1.0.0","mode":"auto"}` |
| `carrito/status/bateria` | ESP publica | `{"voltage":"3.95","level":72}` |
| `carrito/ota/update` | ESP recibe | `{"url":"https://...","version":"v1.1.0"}` |
| `carrito/ota/status` | ESP publica | estado de la actualización |

Valores de `dir`: `forward`, `backward`, `left`, `right`, `stop`

## Características del firmware

- **Evasión autónoma de obstáculos** con HC-SR04 (umbral configurable en `libultrasonic.h`)
- **Control por MQTT/TLS** (broker EMQX en AWS EC2, puerto 8883)
- **OTA** vía HTTPS desde AWS S3
- **Provisioning WiFi** mediante portal cautivo (`CarrIoT-Setup-XXXXXX`)
- **Credenciales WiFi** guardadas en NVS (no hardcodeadas)

## Dashboard web

El directorio `docs/` contiene un dashboard HTML/CSS/JS que se sirve desde
**GitHub Pages** y se conecta al broker mediante WebSocket seguro (WSS, puerto 8884).

Funcionalidades:
- Indicador de distancia en tiempo real con barra de progreso.
- Estado del carrito (IP, versión de firmware, modo, conectividad).
- Selector de modo autónomo / manual.
- Control D-Pad y teclado WASD para el modo manual.
- Ajuste de velocidad con slider.
- Consola de mensajes MQTT en tiempo real.

URL: `https://<usuario>.github.io/carrito-esp32/`

## Configuración

1. Copia `.env.template` a `.env` y completa los valores:
   ```
   WIFI_SSID=MI_RED
   WIFI_PASSWORD=mi_clave
   MQTT_SERVER=carbot.ddnsfree.com
   MQTT_PORT=8883
   MQTT_USER=carrito
   MQTT_PASSWORD=clave_mqtt
   ```

2. Compila y sube:
   ```bash
   pio run -e carrito --target upload
   pio device monitor
   ```

> Si el ESP32-S3 no entra en modo de descarga automáticamente, mantén presionado
> **BOOT**, pulsa y suelta **RESET** y luego suelta **BOOT** antes de iniciar la carga.

## GitHub Actions / CI-CD

El pipeline `.github/workflows/firmware.yml`:
- **Cada push a `main`**: compila el firmware con PlatformIO (`env:carrito`).
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

## Umbral de obstáculo

En `src/libultrasonic.h` se definen dos constantes:

| Constante | Valor por defecto | Descripción |
|---|---|---|
| `OBSTACLE_THRESHOLD_CM` | 25 | Distancia mínima antes de detenerse |
| `CLEAR_PATH_CM` | 40 | Distancia considerada camino libre |

## Estructura del proyecto

```
carrito-esp32/
├── .github/workflows/firmware.yml  # CI/CD — compilar + subir a S3
├── scripts/add_env_defines.py      # Inyección de credenciales en build
├── docs/
│   └── index.html                  # Dashboard web (GitHub Pages)
├── src/
│   ├── main.cpp                    # Punto de entrada + lógica autónoma
│   ├── libiot.h / libiot.cpp       # MQTT / TLS / publicación de estado
│   ├── libmotor.h / libmotor.cpp   # Servos SG90 (LEDC PWM)
│   ├── libultrasonic.h / .cpp      # HC-SR04 — medición de distancia
│   ├── libwifi.h / libwifi.cpp     # Conexión WiFi
│   ├── libstorage.h / libstorage.cpp  # NVS — credenciales persistentes
│   ├── libprovision.h / .cpp       # Portal cautivo de configuración
│   └── libota.h / libota.cpp       # Actualización OTA por HTTPS
├── partitions.csv                  # Tabla de particiones 8 MB (2× OTA)
├── platformio.ini
├── .env.template
└── .gitignore
```
