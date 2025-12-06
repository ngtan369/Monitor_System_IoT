#define ENABLE_DATABASE
#define ENABLE_USER_AUTH

#include "Firebase.h"
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

FirebaseData fbdo;
FirebaseData streamData;
FirebaseAuth auth;
FirebaseConfig config;

SensorData sensordata;
ControlData currentData = {0, 0};


void firebaseTask(void *pvParameters) {
    bool isStreamRunning = false;
    for (;;) {
       if (Firebase.ready()) {
            if (!isStreamRunning) {
                if (Firebase.beginStream(streamData, "system_1/control")) {
                    Serial.println("Stream started inside Task!");
                    isStreamRunning = true; // Đánh dấu là đã chạy xong
                } else {
                    Serial.printf("Stream init failed: %s\n", streamData.errorReason().c_str());
                }
            }
            if (isStreamRunning) {
                if (!Firebase.readStream(streamData)) {
                    Serial.printf("Stream read error: %s\n", streamData.errorReason().c_str());
                    isStreamRunning = false;
                }
                if (streamData.streamAvailable()) {
                    String path = streamData.dataPath();    
                        if (path == "/fan") {
                            currentData.fan = streamData.intData();
                            Serial.println("Stream: Fan updated");
                        } 
                        else if (path == "/led") {
                            currentData.led = streamData.intData();
                            Serial.println("Stream: LED updated");
                        }
                        if (path == "/") {
                            FirebaseJson *json = streamData.jsonObjectPtr();
                           // --- 1. Xử lý Fan (Dùng biến riêng) ---
                            FirebaseJsonData jsonFan; 
                            json->get(jsonFan, "fan"); 
                            if (jsonFan.success) {
                                currentData.fan = jsonFan.intValue; 
                                Serial.printf("JSON update Fan: %d\n", currentData.fan);
                            }

                            // --- 2. Xử lý Led (Dùng biến riêng) ---
                            FirebaseJsonData jsonLed;
                            json->get(jsonLed, "led");
                            if (jsonLed.success) {
                                currentData.led = jsonLed.intValue;
                                Serial.printf("JSON update Led: %d\n", currentData.led);
                            }
                        }
                    if (controlQueue != NULL) {
                        xQueueSend(controlQueue, &currentData, 0);
                    }
                }
            }
        } else {
            Serial.println("Firebase not ready...");
        }
        
        if (xQueueReceive(dataQueue, &sensordata, pdMS_TO_TICKS(1000)) == pdPASS) {
            if (Firebase.ready()) {
                FirebaseJson json;
                json.add("temp", sensordata.temp);
                json.add("humi", sensordata.humi);
                json.add("light", sensordata.light);
                if (Firebase.setJSON(fbdo, "system_1/sensor", json)) {
                    Serial.println("Data sent successfully!");
                } else {
                    Serial.printf("Error: %s\n", fbdo.errorReason().c_str());
                }
            } else {
                Serial.println("Firebase not ready...");
            }
        } 
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void initFirebase() {
    Serial.println("Setting up Firebase...");

    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    config.database_url = DATABASE_URL; 
    config.token_status_callback = tokenStatusCallback;
    
    Firebase.reconnectNetwork(true);
    fbdo.setBSSLBufferSize(4096, 1024); 
    Firebase.begin(&config, &auth);
    Firebase.setDoubleDigits(5);

    xTaskCreatePinnedToCore(
        firebaseTask,
        "firebaseTask",
        10384,
        NULL,
        1,
        NULL,
        1
    );
}