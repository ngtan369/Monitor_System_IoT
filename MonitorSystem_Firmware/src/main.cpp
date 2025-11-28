
#include "globals.h"

void setup() {
    Serial.begin(115200);
    Serial.println("Hello user");
    LittleFS.begin();
    initWiFi();
    // setup_Firebase();
    initLed();
}

void loop() {
    // Wifi_reconnect();
    // vTaskDelay(pdMS_TO_TICKS(50));

    ws.cleanupClients();
    if (needWifiScan) {
        needWifiScan = false;
        WiFi.scanDelete();  
        Serial.println("loop: Scanning WiFi...");
        int n = WiFi.scanNetworks();
        DynamicJsonDocument doc(4096);
        JsonArray arr = doc.createNestedArray("scan");

        for (int i = 0; i < n; i++) {
            JsonObject o = arr.createNestedObject();
            o["ssid"] = WiFi.SSID(i);
            o["rssi"] = WiFi.RSSI(i);
            o["sec"]  = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
        }

        String out;
        serializeJson(doc, out);
        ws.textAll(out);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
}