
#include "globals.h"

void setup() {
    Serial.begin(115200);
    Serial.println("Hello user");
    LittleFS.begin();
    initLed();
    initWiFi();
    setup_Firebase();

}

void loop() {
    WiFi_Handle();
    vTaskDelay(pdMS_TO_TICKS(10));
}