#include "dd_wifi.h"

#include <WiFi.h>

#include "configs.h"
#include "ctrl_stdio.h"

void dd_wifi_init() {
    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
}

bool dd_wifi_connect_blocking(const char* ssid,
                              const char* password,
                              unsigned long timeout_ms) {
    if (ssid == nullptr) {
        return false;
    }
    ctrl_stdio_printf("[WIFI] Conectare la SSID \"%s\" ...\n", ssid);
    WiFi.begin(ssid, password);

    const unsigned long started = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - started >= timeout_ms) {
            ctrl_stdio_printf(
                "[WIFI][EROARE] Timeout (%lu ms) fără asociere\n",
                static_cast<unsigned long>(timeout_ms));
            return false;
        }
        delay(WIFI_RETRY_PERIOD_MS);
        ctrl_stdio_print_text(".");
    }
    ctrl_stdio_print_newline();
    dd_wifi_print_status();
    return true;
}

bool dd_wifi_is_connected() {
    return WiFi.status() == WL_CONNECTED;
}

void dd_wifi_print_status() {
    if (!dd_wifi_is_connected()) {
        ctrl_stdio_print_text("[WIFI] Deconectat\n");
        return;
    }
    const IPAddress ip = WiFi.localIP();
    ctrl_stdio_printf(
        "[WIFI] OK  IP=%u.%u.%u.%u  RSSI=%d dBm\n",
        static_cast<unsigned>(ip[0]),
        static_cast<unsigned>(ip[1]),
        static_cast<unsigned>(ip[2]),
        static_cast<unsigned>(ip[3]),
        static_cast<int>(WiFi.RSSI()));
}
