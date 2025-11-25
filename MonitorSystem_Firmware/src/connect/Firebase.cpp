#define ENABLE_DATABASE
#define ENABLE_USER_AUTH

#include "Firebase.h"
// Định nghĩa các đối tượng Firebase toàn cục (dùng chung)
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// --- HÀM KHỞI TẠO FIREBASE ---
void setup_Firebase() {
    
    // 1. Cấu hình cơ bản
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    // 2. Xác thực Ẩn danh (Anonymous Authentication)
    // Đảm bảo Anonymous Sign-in đã được bật trên Firebase Console
    auth.user.email = ""; // Để trống
    auth.user.password = ""; // Để trống
    config.signer.anonymous = true; 

    // 3. Khởi tạo và Bật Reconnect
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    // 4. Tạo Task gửi dữ liệu
    xTaskCreate(sendDataTask, "FirebaseTask", 4096, NULL, 1, NULL);
    Serial.println("Firebase Client Task đã khởi tạo.");
}

// --- TASK GỬI DỮ LIỆU (FreeRTOS Task) ---
void sendDataTask(void *pvParameters) {
    const TickType_t delayTime = pdMS_TO_TICKS(5000);
    int counter = 0;

    while (true) {
        // 1. Kiểm tra kết nối WiFi và Firebase Ready
        // Firebase.ready() kiểm tra cả WiFi và trạng thái Token
        if (WiFi.isConnected() && Firebase.ready()) {
            counter++;

            // 2. Tạo JSON Payload
            FirebaseJson json;
            float tempValue = 20.0 + (float)random(1, 100) / 10.0;
            float humiValue = 50.0 + (float)random(1, 20) / 10.0;
            
            json.set("temperature", tempValue);
            json.set("humidity", humiValue);

            if (Firebase.RTDB.setJSON(&fbdo, "test", &json)) {
                Serial.printf("Gửi thành công #%d: Temp=%.2f\n", counter, tempValue);
            } else {
                Serial.printf("Lỗi gửi Firebase #%d: %s\n", counter, fbdo.errorReason().c_str());
            }

        } else if (!WiFi.isConnected()){
            Serial.println("Client: Đang chờ kết nối WiFi...");
        } else {
            // Lỗi Token hoặc Initialisation
            Serial.printf("Client: Lỗi Firebase hoặc Token: %s\n", fbdo.errorReason().c_str());
        }
        
        vTaskDelay(delayTime);
    }
}