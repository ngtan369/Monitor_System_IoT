#define ENABLE_DATABASE
#define ENABLE_USER_AUTH

#include "Firebase.h" // File header của bạn
// Đảm bảo 2 dòng này CHỈ nằm ở đây (file .cpp), KHÔNG nằm trong file .h
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

// Khai báo các biến global
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup_Firebase() {
    Serial.println("Setting up Firebase...");

    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    
    // Lưu ý: database_url nên bỏ "https://" và dấu "/" ở cuối cho chuẩn nhất
    // Ví dụ: "du-an-abc.firebaseio.com"
    config.database_url = DATABASE_URL; 

    config.token_status_callback = tokenStatusCallback; // Callback từ TokenHelper

    Firebase.reconnectNetwork(true);
    
    // Tăng buffer nếu gửi dữ liệu lớn (JSON to), với sensor đơn giản thì để mặc định cũng được
    fbdo.setBSSLBufferSize(4096, 1024); 

    Firebase.begin(&config, &auth);
    Firebase.setDoubleDigits(5); // Lấy 5 số sau dấu phẩy cho float
}

void firebaseTask(void *pvParameters) {
    for (;;) {
        // Chỉ gửi khi Firebase đã sẵn sàng và Token hợp lệ
        if (Firebase.ready()) {
            
            // 1. Giả lập số liệu
            float temp = 25.5; 
            float humi = 36.36;

            // 2. Tạo đối tượng JSON bằng thư viện FirebaseJson (nhẹ hơn ArduinoJson)
            FirebaseJson json;
            json.add("temp", temp); // Thêm key "temp"
            json.add("humi", humi); // Thêm key "humi"

            // 3. Gửi lên Firebase
            // Đường dẫn: "system_1/sensors"
            Serial.println("Sending data...");
            
            if (Firebase.setJSON(fbdo, "system_1/sensor", json)) {
                Serial.println("Data sent successfully!");
                // In đường dẫn và ETag để debug nếu cần (xem RTDBHelper)
                // printResult(fbdo); 
            } else {
                Serial.printf("Error: %s\n", fbdo.errorReason().c_str());
            }

        } else {
            Serial.println("Firebase not ready...");
        }

        // Delay 5000ms (5 giây) trước khi lặp lại
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}