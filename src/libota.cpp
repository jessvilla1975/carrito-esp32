#include <libota.h>
#include <libstorage.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <cstring>
#include <cstdlib>

void checkOTAUpdate(const char* payload) {
    Serial.println("[OTA] Mensaje recibido: " + String(payload));
    StaticJsonDocument<512> doc;
    if (deserializeJson(doc, payload)) { Serial.println("[OTA] JSON inválido"); return; }
    if (!doc.containsKey("url")) { Serial.println("[OTA] Sin URL en el payload"); return; }
    const char* url     = doc["url"];
    const char* version = doc["version"] | "desconocida";
    Serial.println("[OTA] Versión actual: " + getFirmwareVersion());
    Serial.println("[OTA] Nueva versión:  " + String(version));
    startOTATask(url, version);
}

void startOTATask(const char* url, const char* version) {
    OTAData* data = (OTAData*)malloc(sizeof(OTAData));
    if (!data) { Serial.println("[OTA] Sin memoria"); return; }
    data->url     = (char*)malloc(strlen(url) + 1);
    data->version = (char*)malloc(strlen(version) + 1);
    if (!data->url || !data->version) {
        free(data->url); free(data->version); free(data);
        Serial.println("[OTA] Sin memoria para strings"); return;
    }
    strcpy(data->url, url);
    strcpy(data->version, version);
    xTaskCreatePinnedToCore(performOTAUpdateTask, "OTA_Task", 8192, data, 1, NULL, 1);
}

void performOTAUpdateTask(void* parameter) {
    OTAData* data = (OTAData*)parameter;

    Update.onProgress([](unsigned int done, unsigned int total) {
        Serial.printf("[OTA] %u/%u bytes (%.0f%%)\r", done, total, (float)done / total * 100.0f);
    });

    Serial.println("\n[OTA] Descargando: " + String(data->url));
    HTTPClient http;
    http.begin(data->url);
    int code = http.GET();
    if (code != HTTP_CODE_OK) {
        Serial.printf("[OTA] HTTP error %d\n", code);
        goto cleanup;
    }
    {
        int size = http.getSize();
        if (!Update.begin(size)) { Serial.println("[OTA] Sin espacio en flash"); goto cleanup; }

        uint8_t buf[OTA_BUFFER_SIZE];
        WiFiClient* stream = http.getStreamPtr();
        size_t written = 0;
        while ((int)written < size) {
            size_t avail = stream->available();
            if (avail) {
                size_t n = stream->readBytes(buf, min(avail, sizeof(buf)));
                if (Update.write(buf, n) != n) { Serial.println("[OTA] Error escritura flash"); goto cleanup; }
                written += n;
            }
            delay(1);
        }

        if (Update.end()) {
            saveFirmwareVersion(String(data->version));
            Serial.println("\n[OTA] OK — reiniciando...");
            http.end();
            free(data->url); free(data->version); free(data);
            delay(500);
            ESP.restart();
        } else {
            Serial.println("[OTA] Error al finalizar: " + String(Update.errorString()));
        }
    }

cleanup:
    http.end();
    free(data->url); free(data->version); free(data);
    vTaskDelete(NULL);
}
