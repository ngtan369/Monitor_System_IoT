#include "Firebase.h"


WiFiClientSecure ssl_client;
FirebaseApp app;
AsyncClient aClient(ssl_client); // Khởi tạo AsyncClient với ssl_client
RealtimeDatabase Database;

// Khởi tạo đối tượng xác thực ẩn danh
AnonymousAuth anon_auth;

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 10000;

int intValue = 0;
float floatValue = 0.01;
String stringValue = "";

void initFirebase() {
    // Cấu hình SSL Client
    ssl_client.setInsecure();
    ssl_client.setConnectionTimeout(1000);
    ssl_client.setHandshakeTimeout(5);

    initializeApp(aClient, app, getAuth(user_auth), processData, "🔐 authTask");
    app.getApp<RealtimeDatabase>(Database);
    Database.url(DATABASE_URL);

    // 4. Tạo Task
    xTaskCreate(test, "FirebaseSend", 4096, NULL, 1, NULL);
}

void test (void * pvParameters){
    while(1){
        // 1. Chạy vòng lặp chính của Firebase để xử lý kết nối, Token, và Events
        app.loop();

        // 2. Kiểm tra nếu App đã sẵn sàng (Auth + Connection)
        if (app.ready()){ 
            unsigned long currentTime = millis();
            if (currentTime - lastSendTime >= sendInterval){
                lastSendTime = currentTime;

                stringValue = "value_" + String(currentTime);
                Database.set<String>(aClient, "/test/string", stringValue, processData, "RTDB_Send_String");
                // send an int
                Database.set<int>(aClient, "/test/int", intValue, processData, "RTDB_Send_Int");
                intValue++; //increment intValue in every loop

                // send a string
                floatValue = 0.01 + random (0,100);
                Database.set<float>(aClient, "/test/float", floatValue, processData, "RTDB_Send_Float");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}

void processData(AsyncResult &aResult) {
    if (!aResult.isResult())
        return;

    if (aResult.isEvent())
        Firebase.printf("Event task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.eventLog().message().c_str(), aResult.eventLog().code());

    if (aResult.isDebug())
        Firebase.printf("Debug task: %s, msg: %s\n", aResult.uid().c_str(), aResult.debug().c_str());

    if (aResult.isError())
        Firebase.printf("Error task: %s, msg: %s, code: %d\n", aResult.uid().c_str(), aResult.error().message().c_str(), aResult.error().code());

    if (aResult.available())
        Firebase.printf("task: %s, payload: %s\n", aResult.uid().c_str(), aResult.c_str());
}