#ifndef __FIREBASE_H__
#define __FIREBASE_H__


#include <Arduino.h>
#include <Firebase_ESP_Client.h> // Thư viện cũ 
#include <WiFi.h>
#include "../../env.h"

extern FirebaseData fbdo;
extern FirebaseAuth auth;
extern FirebaseConfig config;

void setup_Firebase();
void sendDataTask(void *pvParameters);

#endif
