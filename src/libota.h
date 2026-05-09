#ifndef LIBOTA_H
#define LIBOTA_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#define OTA_BUFFER_SIZE 4096

struct OTAData {
    char* url;
    char* version;
};

void checkOTAUpdate(const char* payload);
void performOTAUpdateTask(void* parameter);
void startOTATask(const char* url, const char* version);

#endif /* LIBOTA_H */
