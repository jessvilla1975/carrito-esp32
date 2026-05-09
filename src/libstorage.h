#ifndef LIBSTORAGE_H
#define LIBSTORAGE_H

#include <Arduino.h>

bool saveWiFiCredentials(const String &ssid, const String &password);
bool loadWiFiCredentials(String &outSsid, String &outPassword);
bool clearWiFiCredentials();
bool hasWiFiCredentials();

bool saveFirmwareVersion(const String &version);
bool loadFirmwareVersion(String &outVersion);
String getFirmwareVersion();

#endif /* LIBSTORAGE_H */
