
#include "globals.h"

void setup() {
    Serial.begin(115200);
    Serial.println("Hello user");
    LittleFS.begin();
    
    initWiFi();
    initSensor();
    initControl();
    initFirebase();
}

void loop() {
    WiFi_Handle();
    ws.cleanupClients();

    if (needWifiScan) {
        needWifiScan = false;
        Serial.println("Starting Async WiFi Scan...");
        WiFi.scanNetworks(true); 
    }
  if (needConnectWifi) {
        needConnectWifi = false;
        connectNewWiFi(targetSSID, targetPass);
  }
    int n = WiFi.scanComplete();

    if (n >= 0) {
        Serial.printf("Scan done. Found %d networks.\n", n);
        
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

        WiFi.scanDelete(); 
    }

    vTaskDelay(pdMS_TO_TICKS(50));
}