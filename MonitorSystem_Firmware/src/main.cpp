
#include "globals.h"

void setup() {
    Serial.begin(115200);
    Serial.println("Hello user");
    LittleFS.begin();
    // initWiFi();
    // setup_Firebase();
    test();
    initLed();
}

void loop() {
    // Wifi_reconnect();
    // vTaskDelay(pdMS_TO_TICKS(500));
}