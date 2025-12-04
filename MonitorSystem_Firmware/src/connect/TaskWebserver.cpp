#include "TaskWebserver.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
bool needWifiScan = false;

void sendConfigJson() {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount LittleFS in sendConfigJson");
        return;
    }
    if (!LittleFS.exists("/config.json")) {
        Serial.println("config.json not found");
        return;
    }
    File f = LittleFS.open("/config.json", "r");
    if (!f) {
        Serial.println("Failed to open config.json");
        return;
    }
    String content = f.readString();
    f.close();
    ws.textAll(content);
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

void sendWifiStatus(String msg) {
    DynamicJsonDocument doc(64);
    doc[msg] = true;
    String out;
    serializeJson(doc, out);
    ws.textAll(out);
    Serial.println("TEST 1");
}

void doWifiConnect(String ssid, String pass) {
    WiFi.begin(ssid.c_str(), pass.c_str());
    Serial.println("Connecting...");

    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 8000) {
        vTaskDelay(200);
    }

    if (WiFi.status() == WL_CONNECTED) {
        saveWiFiToFS(ssid, pass);
        sendWifiStatus("wifi_ok"); 
    } else {
        sendWifiStatus("wifi_fail");
    }
}

void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
               AwsEventType type, void* arg, uint8_t* data, size_t len)
{
    if (type == WS_EVT_DATA) {
        AwsFrameInfo* info = (AwsFrameInfo*)arg;
        if (!info->final || info->opcode != WS_TEXT) 
            return;
        String msg = String((char*)data).substring(0, len);
        msg.trim(); // remove newline/whitespace
        Serial.printf("WS_RX from client %u: [%s]\n", client->id(), msg.c_str());
        // --- SCAN WiFi ---
        if (msg == "wifi_scan") {
            Serial.println("Scanning wifi ..."); // for debug
            needWifiScan = true;
        }
        else if (msg.startsWith("wifi_connect:")) {
            DynamicJsonDocument doc(256);
            deserializeJson(doc, msg.substring(13));
            String s = doc["ssid"].as<String>();
            String p = doc["password"].as<String>();
            doWifiConnect(s, p);
        }
        else if (msg == "restart") {
            ESP.restart();
        }
        else if (msg == "get_config") {
            sendConfigJson();
        }
    }
}

void handleRoot(AsyncWebServerRequest* request) {
    request->send(LittleFS, "/index.html", "text/html");
}
//---------------------- OTA from URL ---------------------
bool otaFromUrl(const String &binUrl) {
    HTTPClient http;
    http.begin(binUrl);
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        http.end();
        Serial.printf("OTA: HTTP code %d\n", httpCode);
        return false;
    }

    int contentLength = http.getSize();
    WiFiClient *client = http.getStreamPtr();

    if (!Update.begin(contentLength)) {
        Serial.println("OTA: Update.begin failed");
        http.end();
        return false;
    }

    size_t written = Update.writeStream(*client);
    bool endOk = Update.end();

    http.end();

    if (!endOk || Update.hasError()) {
        Serial.printf("OTA: Update error. written=%u\n", (unsigned)written);
        return false;
    }

    Serial.println("OTA: Update success, restarting...");
    ESP.restart();
    return true; // thực tế sẽ không chạy đến đây vì restart
}

void initWebserver() {
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    server.begin();
    setupOTA(server);
    Serial.println("WebServer + WebSocket started");
}