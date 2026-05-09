# Pre-build: lee .env y define macros C/C++ (credenciales WiFi y MQTT).
# ROOT_CA no se inyecta aquí — está incrustado en libiot.cpp con #ifndef.
Import("env")

import os

PROJECT_DIR = env["PROJECT_DIR"]
ENV_PATH = os.path.join(PROJECT_DIR, ".env")

_MANAGED_DEFINES = frozenset({
    "MQTT_SERVER",
    "MQTT_SERVER_IP",
    "MQTT_USER",
    "MQTT_PASSWORD",
    "WIFI_SSID",
    "WIFI_PASSWORD",
    "MQTT_PORT",
})


def strip_managed_build_flags():
    """Quita -D antiguos de estas claves (caché o configs viejas)."""
    bf = env.get("BUILD_FLAGS", [])
    if isinstance(bf, str):
        bf = [x for x in bf.split() if x]
    elif not bf:
        bf = []
    new_bf = []
    for flag in bf:
        if not isinstance(flag, str):
            new_bf.append(flag)
            continue
        stripped = flag.strip()
        skip = any(
            stripped.startswith("-D " + n + "=") or stripped.startswith("-D" + n + "=")
            for n in _MANAGED_DEFINES
        )
        if not skip:
            new_bf.append(flag)
    env["BUILD_FLAGS"] = new_bf

    cpdefs = env.get("CPPDEFINES", [])
    if not cpdefs:
        return
    cleaned = []
    for item in cpdefs:
        if isinstance(item, tuple) and item[0] in _MANAGED_DEFINES:
            continue
        if isinstance(item, str):
            if any(item.startswith(n + "=") or item == n for n in _MANAGED_DEFINES):
                continue
        cleaned.append(item)
    env.Replace(CPPDEFINES=cleaned)


def load_dotenv(path):
    """Carga .env en os.environ."""
    if not os.path.isfile(path):
        return
    with open(path, encoding="utf-8") as f:
        for raw in f:
            line = raw.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue
            key, _, val = line.partition("=")
            key = key.strip()
            val = val.strip()
            if (val.startswith('"') and val.endswith('"')) or \
               (val.startswith("'") and val.endswith("'")):
                val = val[1:-1]
            if key:
                os.environ[key] = val


def env_str(key, default=""):
    v = os.environ.get(key, "").strip()
    return v if v else default


def append_str_macro(name, value):
    esc = value.replace("\\", "\\\\").replace('"', '\\"')
    env.Append(BUILD_FLAGS=['-D {}=\\"{}\\"'.format(name, esc)])


strip_managed_build_flags()
load_dotenv(ENV_PATH)

mqtt_server    = env_str("MQTT_SERVER")
mqtt_server_ip = env_str("MQTT_SERVER_IP")
mqtt_user      = env_str("MQTT_USER",     "carrito")
mqtt_password  = env_str("MQTT_PASSWORD")
wifi_ssid      = env_str("WIFI_SSID")
wifi_password  = env_str("WIFI_PASSWORD")

port_str = env_str("MQTT_PORT", "8883")
try:
    mqtt_port = int(port_str)
except ValueError:
    mqtt_port = 8883

append_str_macro("MQTT_SERVER",    mqtt_server)
append_str_macro("MQTT_SERVER_IP", mqtt_server_ip)
append_str_macro("MQTT_USER",      mqtt_user)
append_str_macro("MQTT_PASSWORD",  mqtt_password)
append_str_macro("WIFI_SSID",      wifi_ssid)
append_str_macro("WIFI_PASSWORD",  wifi_password)
env.Append(BUILD_FLAGS=["-D MQTT_PORT={}".format(mqtt_port)])

print("[add_env_defines] MQTT_SERVER={!r}  MQTT_PORT={}".format(mqtt_server, mqtt_port))
print("[add_env_defines] WIFI_SSID={!r}".format(wifi_ssid))
print("[add_env_defines] ROOT_CA se usa el valor por defecto en libiot.cpp")
