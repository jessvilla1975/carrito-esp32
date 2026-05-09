#include <WiFi.h>
#include <WebServer.h>
#include <libstorage.h>
#include <libprovision.h>

static WebServer server(80);
static bool s_isProvisioning = false;

static const char FORM_HTML[] PROGMEM = R"HTML(
<!doctype html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>
<style>
  body{font-family:sans-serif;margin:24px;background:#f5f5f5}
  h3{color:#333}
  input,button{font-size:16px;padding:10px;margin:6px 0;width:100%;box-sizing:border-box;border-radius:6px;border:1px solid #ccc}
  button{background:#007bff;color:#fff;border:none;cursor:pointer}
  button:hover{background:#0056b3}
  form{max-width:420px;background:#fff;padding:20px;border-radius:10px;box-shadow:0 2px 8px rgba(0,0,0,.1)}
</style>
</head><body>
  <h3>CarrIoT — Configurar Wi-Fi</h3>
  <form method='POST' action='/save'>
    <label>SSID</label>
    <input name='ssid' required placeholder='Nombre de la red'>
    <label>Contraseña</label>
    <input name='password' type='password' placeholder='Contraseña WiFi'>
    <button type='submit'>Guardar y reiniciar</button>
  </form>
</body></html>
)HTML";

static void handleRoot() { server.send(200, "text/html", FORM_HTML); }

static void handleSave() {
    if (!server.hasArg("ssid")) { server.send(400, "text/plain", "ssid requerido"); return; }
    String ssid = server.arg("ssid");
    String pwd  = server.arg("password");
    if (!saveWiFiCredentials(ssid, pwd)) { server.send(500, "text/plain", "Error al guardar"); return; }
    server.send(200, "text/plain", "Guardado. Reiniciando en 2 segundos...");
    delay(2000);
    ESP.restart();
}

void startProvisioningAP() {
    WiFi.mode(WIFI_AP);
    String apName = String("CarrIoT-Setup-") + String((uint32_t)ESP.getEfuseMac(), HEX);
    WiFi.softAP(apName.c_str());
    IPAddress ip = WiFi.softAPIP();
    Serial.print("AP: "); Serial.print(apName);
    Serial.print("  IP: "); Serial.println(ip);
    server.on("/", HTTP_GET, handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.begin();
    s_isProvisioning = true;
}

void provisioningLoop() { if (s_isProvisioning) server.handleClient(); }
bool isProvisioning()   { return s_isProvisioning; }
