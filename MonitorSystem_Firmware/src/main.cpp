
#include "globals.h"

void setup() {
    Serial.begin(115200);
    Serial.println("Hello user");
    LittleFS.begin();
    initWiFi();
    initWebserver();
    initFirebase();
    initLed();
}

void loop() {
    Wifi_reconnect();
    delay(2000);
}