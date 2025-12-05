
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
    vTaskDelay(pdMS_TO_TICKS(10));
}