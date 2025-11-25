#ifndef INC_TASKWIFI_H_
#define INC_TASKWIFI_H_

#include <HTTPClient.h>
#include <LittleFS.h>
#include <WiFi.h>
#include "TaskWebserver.h"
#include "../device/TaskLed.h"

#define FLASH_DELAY_NO_WIFI 4000     
#define FLASH_DELAY_AP_MODE 1000
#define FLASH_DELAY_NO_INTERNET 2000
#define BOOT_BUTTON 0

extern String ssid;
extern String password;

void test();
void initWiFi();
void Wifi_reconnect();
#endif /* INC_TASKWIFI_H_ */