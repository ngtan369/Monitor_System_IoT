#ifndef TASK_WEBSERVER_H
#define TASK_WEBSERVER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Update.h>
#include "TaskWifi.h"

extern String targetSSID;
extern String targetPass;
extern bool  needWifiScan;
extern bool  needConnectWifi;
extern AsyncWebSocket ws;
void initWebserver();
void connectNewWiFi(String, String);
#endif