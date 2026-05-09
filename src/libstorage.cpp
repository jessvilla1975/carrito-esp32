#include <Arduino.h>
#include <Preferences.h>
#include <libstorage.h>

static const char* kNamespace          = "cred";
static const char* kWiFiSsidKey        = "wifi_ssid";
static const char* kWiFiPwdKey         = "wifi_pwd";
static const char* kFirmwareVersionKey = "fw_version";

bool saveWiFiCredentials(const String &ssid, const String &password) {
    if (ssid.length() == 0) return false;
    Preferences prefs;
    if (!prefs.begin(kNamespace, false)) return false;
    bool ok = prefs.putString(kWiFiSsidKey, ssid) > 0;
    ok = ok && (prefs.putString(kWiFiPwdKey, password) >= 0);
    prefs.end();
    return ok;
}

bool loadWiFiCredentials(String &outSsid, String &outPassword) {
    Preferences prefs;
    if (!prefs.begin(kNamespace, true)) return false;
    if (!prefs.isKey(kWiFiSsidKey)) { prefs.end(); return false; }
    String s = prefs.getString(kWiFiSsidKey, "");
    String p = prefs.getString(kWiFiPwdKey,  "");
    prefs.end();
    if (s.length() == 0) return false;
    outSsid = s; outPassword = p;
    return true;
}

bool clearWiFiCredentials() {
    Preferences prefs;
    if (!prefs.begin(kNamespace, false)) return false;
    bool ok = prefs.remove(kWiFiSsidKey);
    ok = prefs.remove(kWiFiPwdKey) || ok;
    prefs.end();
    return ok;
}

bool hasWiFiCredentials() {
    Preferences prefs;
    if (!prefs.begin(kNamespace, true)) return false;
    if (!prefs.isKey(kWiFiSsidKey)) { prefs.end(); return false; }
    String s = prefs.getString(kWiFiSsidKey, "");
    prefs.end();
    return s.length() > 0;
}

bool saveFirmwareVersion(const String &version) {
    if (version.length() == 0) return false;
    Preferences prefs;
    if (!prefs.begin(kNamespace, false)) return false;
    bool ok = prefs.putString(kFirmwareVersionKey, version) > 0;
    prefs.end();
    return ok;
}

bool loadFirmwareVersion(String &outVersion) {
    Preferences prefs;
    if (!prefs.begin(kNamespace, true)) return false;
    String v = prefs.getString(kFirmwareVersionKey, "");
    prefs.end();
    if (v.length() == 0) return false;
    outVersion = v;
    return true;
}

String getFirmwareVersion() {
    String version;
    if (loadFirmwareVersion(version)) return version;
#ifndef FIRMWARE_VERSION
    String def = "v1.0.0";
#else
    String def = String(FIRMWARE_VERSION);
#endif
    saveFirmwareVersion(def);
    return def;
}
