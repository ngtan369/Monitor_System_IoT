
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

    // 1. Kích hoạt quét (Chỉ gửi lệnh rồi thoát ngay, không chờ)
    if (needWifiScan) {
        needWifiScan = false;
        Serial.println("Starting Async WiFi Scan...");
        // Tham số 'true' = Async Mode (Chạy ngầm)
        WiFi.scanNetworks(true); 
    }
  if (needConnectWifi) {
        needConnectWifi = false;
        connectNewWiFi(targetSSID, targetPass);
  }
    // 2. Kiểm tra kết quả quét trong mỗi vòng lặp
    // scanComplete() trả về:
    // -2: Chưa bắt đầu quét
    // -1: Đang quét (Busy)
    // >= 0: Số mạng tìm thấy (Đã xong)
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