#ifndef __FIREBASE_H__
#define __FIREBASE_H__


#include <Arduino.h>
#include <FirebaseESP32.h>
#include <WiFi.h>
#include "../../env.h"


void setup_Firebase();
void sendDataTask(void *pvParameters);

#endif
