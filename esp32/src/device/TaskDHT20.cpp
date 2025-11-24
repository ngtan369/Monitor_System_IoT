#include "TaskDHT20.h"

DHT20 dht20;
float temp;
float humi;
void TaskDHT20(void* pvParameters) {
    while (true) {
        if (dht20.read() == DHT20_OK) {
            temp = dht20.getTemperature();
            humi = dht20.getHumidity();
            // FirebaseJson json;
            // json.set("temperature", temp);
            // json.set("humidity", humi);
            // if (Firebase.ready()) {
            //     if (Firebase.RTDB.setJSON(&fbdo, "/system1/sensor", &json)) {
            //         Serial.println("Dữ liệu đã gửi lên Firebase thành công.");
            //     } else {
            //         Serial.print("Lỗi gửi Firebase: ");
            //         Serial.println(fbdo.errorReason());
            //     }
            // }
            Serial.print("temp: ");
            Serial.print(temp);
            Serial.print("\t humi: ");
            Serial.println(humi);
        } else {
            Serial.println("Failed to read DHT20 sensor.");
        }
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void initDHT20() {
    Wire.begin(SDA, SCL);
    dht20.begin();
    xTaskCreate(TaskDHT20, "TaskDHT20", 4096, NULL, 1, NULL);
}