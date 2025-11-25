#include "TaskWifi.h"

constexpr char AP_SSID[] = "ESP32_CONFIG";
constexpr char AP_PASSWORD[] = "87654321";

String ssid = "";
String password = "";

bool ap_mode = false;
bool checkInternet(unsigned long timeoutMs = 3000) {
    HTTPClient http;
    http.setTimeout(timeoutMs);
    if (!http.begin("http://clients3.google.com/generate_204")) {
        http.end();
        return false;
    }
    int httpCode = http.GET();
    http.end();
    return (httpCode == 204);
}

bool loadWiFiFromFS() {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount LittleFS");
        return false;
    }
    if (!LittleFS.exists("/wifi.json")) {
        Serial.println("wifi.json does not exist");
        return false;
    }
    File file = LittleFS.open("/wifi.json", "r");
    if (!file) {
        Serial.println("Failed to open wifi.json for reading");
        return false;
    }
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error) {
        Serial.println("Failed to parse wifi.json");
        return false;
    }
    ssid = doc["ssid"].as<String>();
    password = doc["password"].as<String>();
    Serial.println("WiFi credentials loaded from FS");
    return true;
}

void setup_AP() {
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
}

void setup_STA() {
    Serial.println("Connecting to WiFi ...");
    WiFi.begin(ssid, password);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < 7000) {
        Serial.print("...");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to AP");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }
}

void initWiFi() {
    loadWiFiFromFS();
    pinMode(BOOT_BUTTON, INPUT_PULLUP);
    WiFi.mode(WIFI_STA);
    setup_STA();
}

void test(){
    WiFi.mode(WIFI_AP);
    setup_AP();
    initWebserver(); 
}
void Wifi_reconnect() {
    if (digitalRead(BOOT_BUTTON) == 0) {
        uint32_t timepress = millis();
        while (digitalRead(BOOT_BUTTON) == 0) {
            if (millis() - timepress >= 3000) {
                ap_mode = true;
                Serial.println("Vào chế độ cấu hình AP");
                WiFi.disconnect(true);
                WiFi.mode(WIFI_AP);
                setup_AP();
                initWebserver();
                xTaskNotify(xTaskLedHandle, FLASH_DELAY_AP_MODE, eSetValueWithOverwrite);
                return;
            }
            vTaskDelay(10);
        }
    }
    if (ap_mode) {
        return;
    }

    const wl_status_t status = WiFi.status();
    uint32_t ulTargetDelay = 0;

    if (status != WL_CONNECTED) {
        ulTargetDelay = FLASH_DELAY_NO_WIFI;
    }
    else if (!checkInternet()) {
        ulTargetDelay = FLASH_DELAY_NO_INTERNET;
    }
    else {
        ulTargetDelay = 0;
    }

    if (xTaskLedHandle != NULL) {
        xTaskNotify(xTaskLedHandle, ulTargetDelay, eSetValueWithOverwrite);
    }
}
