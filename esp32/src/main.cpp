
#include "globals.h"

void setup() {
    Serial.begin(115200);
    Serial.println("Hello user");
    LittleFS.begin();
    initWiFi();
    initWebserver();
    // initFirebase();
    initDHT20();
    initLed();
    initFan();
}

void loop() {
    Wifi_reconnect();
    delay(500);
}