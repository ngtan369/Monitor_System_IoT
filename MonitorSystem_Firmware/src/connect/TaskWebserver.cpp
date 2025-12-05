#include "TaskWebserver.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
bool needWifiScan = false;
String ota_version_link = "https://raw.githubusercontent.com/TrungTan369/Embedded_System_Course/main/latest.json";
String current_version = "0.0";
String latest_Version = "";
String ota_update_link = "";

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
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, content);
    if (!err) {
        const char* fw = doc["version"];
        if (fw) {
            current_version = fw;
        }
    }
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
bool saveVersionToFS(){
    DynamicJsonDocument doc(256);
    doc["version"] = latest_Version;

    File file = LittleFS.open("/config.json", "r");
    if (!file) {
        Serial.println("Failed to open config.json for reading");
        return false;
    }
    DeserializationError err = deserializeJson(doc, file);
    file.close();
    if (err) {
        Serial.println("Failed to parse config.json");
        return false;
    }
    doc["version"] = latest_Version;

    file = LittleFS.open("/config.json", "w");
    if (!file) {
        Serial.println("Failed to open config.json for writing");
        return false;
    }
    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write to config.json");
        file.close();
        return false;
    }
    file.close();
    Serial.println("Version info saved to FS");
    return true;
}
void SendMsgToWeb(String msg) {
    DynamicJsonDocument doc(64);
    doc[msg] = true;
    String out;
    serializeJson(doc, out);
    ws.textAll(out);
}

void scan_wifi(String ssid, String pass) {
    WiFi.begin(ssid.c_str(), pass.c_str());
    Serial.println("Connecting...");

    unsigned long t0 = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - t0 < 8000) {
        vTaskDelay(200);
    }

    if (WiFi.status() == WL_CONNECTED) {
        saveWiFiToFS(ssid, pass);
        SendMsgToWeb("wifi_ok"); 
    } else {
        SendMsgToWeb("wifi_fail");
    }
}

bool checkAndReportLatestVersion() {
    if (WiFi.status() != WL_CONNECTED) 
        return false;
    HTTPClient http;
    http.begin(ota_version_link);
    int code = http.GET();
    if (code != HTTP_CODE_OK) { 
        http.end();
        Serial.println("OTA check link failed");
        return false; 
    } else {
        Serial.println("OTA check link OK");
    }

    String payload = http.getString();
    http.end();
    StaticJsonDocument<512> doc;
    if (deserializeJson(doc, payload)) 
        return false;
    const char* latestVer = doc["ver"];
    const char * url      = doc["url"];
    const char* note      = doc["note"];
    const char* createAt  = doc["createAt"];
    ota_update_link = url;
    latest_Version = latestVer;
    // gửi thông tin lên WebSocket cho UI
    StaticJsonDocument<256> out;
    out["cur_ver"]      = current_version;
    out["latest_ver"]   = latestVer;
    out["note"]         = note;
    out["createAt"]     = createAt;
    String msg;
    serializeJson(out, msg);
    ws.textAll(msg);
    return true;
}

//---------------------- OTA from URL ---------------------
bool otaFromUrl(const String &binUrl) {
    const int MAX_REDIRECTS = 5;
    int redirects = 0;
    String url = binUrl;

    HTTPClient http;
    int httpCode = 0;

    // follow redirects up to MAX_REDIRECTS
    while (true) {
        Serial.print("OTA: GET ");
        Serial.println(url);

        if (!http.begin(url)) {
            Serial.println("OTA: http.begin failed");
            SendMsgToWeb("update_fail");
            return false;
        }

        httpCode = http.GET();

        // success
        if (httpCode == HTTP_CODE_OK) {
            Serial.println("OTA: HTTP 200 OK");
            break;
        }

        // redirect
        if (httpCode >= 300 && httpCode < 400) {
            String location = http.getLocation();
            http.end();
            if (location.length() == 0) {
                Serial.println("OTA: redirect but no Location header");
                SendMsgToWeb("update_fail");
                return false;
            }
            url = location;
            redirects++;
            Serial.printf("OTA: redirect -> %s (count=%d)\n", url.c_str(), redirects);
            if (redirects > MAX_REDIRECTS) {
                Serial.println("OTA: too many redirects");
                SendMsgToWeb("update_fail");
                return false;
            }
            // loop to follow the new location
            continue;
        }

        // other errors
        Serial.printf("OTA: HTTP code %d\n", httpCode);
        http.end();
        SendMsgToWeb("update_fail");
        return false;
    }

    int contentLength = http.getSize();
    WiFiClient *client = http.getStreamPtr();

    // begin update (support unknown size)
    if (contentLength > 0) {
        Serial.printf("OTA: contentLength=%d\n", contentLength);
        if (!Update.begin(contentLength)) {
            Serial.println("OTA: Update.begin failed (insufficient space?)");
            http.end();
            SendMsgToWeb("update_fail");
            return false;
        }
    } else {
        Serial.println("OTA: content length unknown, using UPDATE_SIZE_UNKNOWN");
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Serial.println("OTA: Update.begin failed (unknown size)");
            http.end();
            SendMsgToWeb("update_fail");
            return false;
        }
    }

    size_t written = Update.writeStream(*client);
    bool endOk = Update.end();
    http.end();

    Serial.printf("OTA: written=%u\n", (unsigned)written);

    if (!endOk || Update.hasError()) {
        Serial.println("OTA: Update error");
        SendMsgToWeb("update_fail");
        return false;
    }

    Serial.println("OTA: Update success, restarting...");
    SendMsgToWeb("update_ok");
    saveVersionToFS(); // new version saved to FS
    vTaskDelay(pdMS_TO_TICKS(5000));
    ESP.restart();
    return true; // not reached
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
            scan_wifi(s, p);
        }
        else if (msg == "restart") {
            ESP.restart();
        }
        else if (msg == "get_config") {
            sendConfigJson();
        }else if (msg == "ota_check") {
            checkAndReportLatestVersion();
        } else if (msg == "ota_update") {
            otaFromUrl(ota_update_link);
        }
    }
}

void handleRoot(AsyncWebServerRequest* request) {
    request->send(LittleFS, "/index.html", "text/html");
}

void initWebserver() {
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    server.begin();
    Serial.println("WebServer + WebSocket started");
}