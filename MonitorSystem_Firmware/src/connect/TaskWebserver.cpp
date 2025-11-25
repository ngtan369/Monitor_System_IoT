#include "TaskWebserver.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

String getSensorJson() {
    String json = "{";
    // json += "\"temp\":" + String(10) + ",";
    // json += "\"humi\":" + String(10) + ",";
    // json += "\"soil\":" + String(10) + ",";
    // json += "\"distance\":" + String(10) + ",";
    // json += "\"light\":" + String(10) + ",";
    // json += "}";
    return json;
}

bool saveWiFiToFS(const String& ssid, const String& password) {
    DynamicJsonDocument doc(256);
    doc["ssid"] = ssid;
    doc["password"] = password;

    File file = LittleFS.open("/wifi.json", "w");
    if (!file) {
        Serial.println("Failed to open wifi.json for writing");
        return false;
    }
    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write to wifi.json");
        file.close();
        return false;
    }
    file.close();
    Serial.println("WiFi credentials saved to FS");
    return true;
}

void sendRestartPopup() {
    DynamicJsonDocument doc(64);
    doc["wifi_ok"] = true;
    String out;
    serializeJson(doc, out);
    ws.textAll(out);
}

void doWifiConnect(String ssid, String pass) {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(200);

    WiFi.begin(ssid.c_str(), pass.c_str());
    Serial.println("Connecting...");

    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 8000) {
        delay(200);
    }

    if (WiFi.status() == WL_CONNECTED) {
        saveWiFiToFS(ssid, pass);
        sendRestartPopup();  // gửi popup hỏi restart
    }
}

void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
               AwsEventType type, void* arg, uint8_t* data, size_t len)
{
    if (type == WS_EVT_CONNECT) {
        client->text(getSensorJson());
        return;
    }
    if (type == WS_EVT_DATA) {
        AwsFrameInfo* info = (AwsFrameInfo*)arg;
        if (!info->final || info->opcode != WS_TEXT) return;
        String msg = String((char*)data).substring(0, len);
        // --- SCAN WiFi ---
        if (msg == "wifi_scan") {
            int n = WiFi.scanNetworks();
            DynamicJsonDocument doc(2048);
            JsonArray arr = doc.createNestedArray("scan");

            for (int i = 0; i < n; i++) {
                JsonObject o = arr.createNestedObject();
                o["ssid"] = WiFi.SSID(i);
                o["rssi"] = WiFi.RSSI(i);
                o["sec"]  = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
            }

            String out;
            serializeJson(doc, out);
            client->text(out);
            return;
        }

        // --- Kết nối WiFi ---
        if (msg.startsWith("wifi_connect:")) {
            DynamicJsonDocument doc(256);
            deserializeJson(doc, msg.substring(13));
            String s = doc["ssid"].as<String>();
            String p = doc["password"].as<String>();
            doWifiConnect(s, p);
            return;
        }
        // --- Restart yes ---
        if (msg == "restart_yes") {
            ESP.restart();
        }
        if (msg == "get_data") client->text(getSensorJson());
    }
}

void handleRoot(AsyncWebServerRequest* request) {
    request->send(LittleFS, "/index.html", "text/html");
}

void setupOTA(AsyncWebServer& server) {
    server.on("/update", HTTP_POST, [](AsyncWebServerRequest* request) {
        bool shouldReboot = !Update.hasError();
        request->send(200, "text/html", shouldReboot ? "OK" : "FAIL");
        if (shouldReboot) {
            delay(1000);
            ESP.restart();
        } }, [](AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
        if (!index) {
            Update.begin(UPDATE_SIZE_UNKNOWN);
        }
        Update.write(data, len);
        if (final) {
            Update.end(true);
        } });
}

void initWebserver() {
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    server.begin();
    setupOTA(server);
    Serial.println("WebServer + WebSocket started");
}