#ifndef LIBWIFI_H
#define LIBWIFI_H

extern const char* ssid;
extern const char* password;

void listWiFiNetworks();
void startWiFi(const char* hostname);
void checkWiFi();
bool hasStoredWiFi();
bool saveWiFi(const String &ssid, const String &pwd);
bool clearStoredWiFi();
void factoryReset();

#endif /* LIBWIFI_H */
