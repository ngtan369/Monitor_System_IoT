#include "TaskWifi.h"

constexpr char AP_SSID[] = "ESP32_CONFIG";
constexpr char AP_PASSWORD[] = "87654321";

String ssid = "";
String password = "";
TaskHandle_t Task_Led_Indicate_NO_WIFI_Handle = NULL;
TaskHandle_t Task_Led_Indicate_AP_MODE_Handle = NULL;
TaskHandle_t Task_Led_Indicate_NO_INTERNET_Handle = NULL;
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
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < 3000) {
        Serial.print("...");
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to AP");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("Cannot connect to WIFI");
    }
}

void initWiFi() {
    loadWiFiFromFS();
    pinMode(BOOT_BUTTON, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    WiFi.mode(WIFI_STA);
    setup_STA();
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
                return;
            }
            delay(10);
        }
    }
    if (ap_mode) {
        // --- AP MODE ---
        // Gửi Notification với giá trị delay cho AP_MODE
        if (xTaskLedHandle != NULL) {
            xTaskNotify(xTaskLedHandle, FLASH_DELAY_AP_MODE, eSetValueWithOverwrite);
        }
        return;
    }

    const wl_status_t status = WiFi.status();
    uint32_t ulTargetDelay = 0;  // Giá trị delay cần gửi

    if (status != WL_CONNECTED) {
        // --- NO WIFI ---
        ulTargetDelay = FLASH_DELAY_NO_WIFI;
        Serial.println("NO WIFI -> LED FLASH");
    } else if (status == WL_CONNECTED && !checkInternet()) {
        // --- WIFI BUT NO INTERNET ---
        ulTargetDelay = FLASH_DELAY_NO_INTERNET;
        Serial.println("NO INTERNET -> LED FLASH");
    } else {
        // --- WIFI CONNECTED & INTERNET OK (Task LED nên tắt hoặc bật cố định)
        // Ta có thể gửi giá trị 0ms để tắt hoặc một giá trị lớn để bật cố định (vd: 0xffffffff)
        // Tùy chọn: Gửi 0ms và Task LED sẽ xử lý để dừng/tắt LED
        ulTargetDelay = 0;  // Giả sử 0ms nghĩa là Tắt LED
        led_off();          // Tắt luôn LED
        Serial.println("CONNECTED -> LED OFF");
    }

    // Gửi Notification cho Task LED với chu kỳ delay mới
    if (xTaskLedHandle != NULL && ulTargetDelay > 0) {
        // eSetValueWithOverwrite: Ghi đè nếu có thông báo đang chờ
        xTaskNotify(xTaskLedHandle, ulTargetDelay, eSetValueWithOverwrite);
    }
    // Nếu ulTargetDelay == 0, ta đã gọi led_off() và Task LED sẽ không cần nháy nữa
    // Cách xử lý Task LED khi Connected (ulTargetDelay=0) cần thêm logic:
    // Ví dụ, Task LED nên tự kiểm tra nếu giá trị nhận được là 0 thì gọi vTaskSuspend()
}
