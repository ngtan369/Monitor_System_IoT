#define ENABLE_DATABASE
#define ENABLE_USER_AUTH

#include "Firebase.h" // File header của bạn
// Đảm bảo 2 dòng này CHỈ nằm ở đây (file .cpp), KHÔNG nằm trong file .h
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// Khai báo các biến global
FirebaseData fbdo;
FirebaseData streamData; // Để NHẬN (Downlink)
FirebaseAuth auth;
FirebaseConfig config;

SensorData sensordata;
ControlData currentData = {0.0, 0.0};

void firebaseTask(void *pvParameters) {

    for (;;) {
       if (Firebase.ready()) {
            if (Firebase.beginStream(streamData, "system_1/control")) {
                Serial.println("Stream started inside Task!");
            } else {
                    Serial.printf("Stream init failed: %s\n", streamData.errorReason().c_str());
            }
            if (!Firebase.readStream(streamData)) {
                Serial.printf("Stream read error: %s\n", streamData.errorReason().c_str());
            }
            if (streamData.streamAvailable()) {
                String path = streamData.dataPath();    
                if (path == "/fan") {
                    currentData.fan = streamData.floatData();
                    Serial.println("Stream: Fan updated");
                } 
                else if (path == "/led") {
                    currentData.led = streamData.floatData();
                    Serial.println("Stream: LED updated");
                }
                if (path == "/") {
                    FirebaseJson *json = streamData.jsonObjectPtr();
                }
                if (controlQueue != NULL) {
                    xQueueSend(controlQueue, &currentData, 0);
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

    // if (!Firebase.beginStream(streamData, "system_1/control")) {
    //     Serial.printf("Stream begin error: %s\n", streamData.errorReason().c_str());
    // } else {
    //     Serial.println("Stream started successfully!");
    // }

    xTaskCreate(
        firebaseTask,
        "firebaseTask",
        8192,
        NULL,
        1,
        NULL
    );
}