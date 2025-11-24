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

void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type,
               void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        client->text(getSensorJson());
    } else if (type == WS_EVT_DATA) {
        AwsFrameInfo* info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            String msg = String((char*)data);
            if (msg == "get_data") {
                client->text(getSensorJson());
            } else if (msg.startsWith("wifi:")) {
                DynamicJsonDocument doc(256);
                DeserializationError err = deserializeJson(doc, msg.substring(5));
                if (!err) {
                    ssid = doc["ssid"].as<String>();
                    password = doc["password"].as<String>();
                    saveWiFiToFS(ssid, password);
                    ESP.restart();
                }
            }
        }
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