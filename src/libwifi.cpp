#include <WiFi.h>
#include <libwifi.h>
#include <libstorage.h>
#include <Arduino.h>
#include "esp_netif.h"

static void applyPublicDnsOverride();

static bool waitForWiFiConnection(int maxAttempts = 20) {
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
        delay(500); Serial.print("."); attempts++;
    }
    return WiFi.status() == WL_CONNECTED;
}

static bool connectWiFiWithCredentials(const char* targetSsid, const char* targetPassword, const char* label) {
    if (targetSsid == nullptr || strlen(targetSsid) == 0) return false;
    Serial.println(label);
    WiFi.disconnect(false, true);
    delay(200);
    WiFi.begin(targetSsid, targetPassword);
    if (!waitForWiFiConnection()) { Serial.println("\nWiFi connection failed"); return false; }
    Serial.println("\nWiFi connected");
    Serial.print("SSID: ");      Serial.println(WiFi.SSID());
    Serial.print("IP address: "); Serial.println(WiFi.localIP());
    WiFi.setSleep(false);
    applyPublicDnsOverride();
    return true;
}

/*
 * El DHCP del campus a veces entrega DNS que "secuestran" dominios.
 * Forzar DNS públicos evita conectar MQTT a una IP equivocada.
 */
static void applyPublicDnsOverride() {
    esp_netif_t* sta = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (!sta) return;
    esp_netif_dns_info_t mainDns{};
    mainDns.ip.type = ESP_IPADDR_TYPE_V4;
    mainDns.ip.u_addr.ip4.addr = esp_ip4addr_aton("8.8.8.8");
    esp_netif_dns_info_t backupDns{};
    backupDns.ip.type = ESP_IPADDR_TYPE_V4;
    backupDns.ip.u_addr.ip4.addr = esp_ip4addr_aton("1.1.1.1");
    esp_err_t e1 = esp_netif_set_dns_info(sta, ESP_NETIF_DNS_MAIN,   &mainDns);
    esp_err_t e2 = esp_netif_set_dns_info(sta, ESP_NETIF_DNS_BACKUP, &backupDns);
    if (e1 == ESP_OK && e2 == ESP_OK)
        Serial.println("DNS: 8.8.8.8 (main) 1.1.1.1 (backup)");
}

void checkWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi perdido. Reconectando...");
        WiFi.reconnect();
        if (waitForWiFiConnection()) {
            WiFi.setSleep(false);
            applyPublicDnsOverride();
        } else {
            startWiFi("");
        }
    }
}

void listWiFiNetworks() {
    Serial.println("Escaneando redes WiFi...");
    int n = WiFi.scanNetworks();
    if (n == 0) { Serial.println("No se encontraron redes"); return; }
    Serial.print(n); Serial.println(" redes encontradas:");
    for (int i = 0; i < n; ++i) {
        Serial.printf("  %d: %s (%d dBm)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    }
    WiFi.scanDelete();
}

void startWiFi(const char* hostname) {
    if (hostname && strlen(hostname) > 0) WiFi.setHostname(hostname);
    String s, p;
    bool connected = false;
    if (loadWiFiCredentials(s, p)) {
        connected = connectWiFiWithCredentials(s.c_str(), p.c_str(), "Credenciales guardadas (NVS)");
    }
    if (!connected) {
        connected = connectWiFiWithCredentials(ssid, password, "Credenciales de compilación");
        if (connected) saveWiFiCredentials(String(ssid), String(password));
    }
}

bool hasStoredWiFi()                      { return hasWiFiCredentials(); }
bool saveWiFi(const String &s, const String &pwd) { return saveWiFiCredentials(s, pwd); }
bool clearStoredWiFi()                    { return clearWiFiCredentials(); }

void factoryReset() {
    Serial.println("Factory reset: limpiando credenciales...");
    clearWiFiCredentials();
    WiFi.disconnect(true, true);
    delay(500);
    ESP.restart();
}
