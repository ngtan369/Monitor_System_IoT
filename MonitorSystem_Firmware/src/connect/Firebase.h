#ifndef INC_TASKMQTT_H_
#define INC_TASKMQTT_H_

#define ENABLE_USER_AUTH
#define ENABLE_DATABASE

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include "../env.h"

// --- Các biến toàn cục (extern) ---
WiFiClientSecure ssl_client;
FirebaseApp app;
AsyncClient aClient;
RealtimeDatabase Database;

// Dùng cho xác thực ẩn danh
AnonymousAuth anon_auth;

unsigned long lastSendTime;
const unsigned long sendInterval;

// --- Khai báo hàm ---
void initFirebase();
void test(void *pvParameters);
void processData(AsyncResult &aResult);

#endif /* INC_TASKMQTT_H_ */