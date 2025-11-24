#include "Firebase.h"

#include "addons/RTDBHelper.h"
#include "addons/TokenHelper.h"
String api_key = API_KEY;
String database_url = DATABASE_URL;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void initFirebase() {
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;

    config.signer.anonymous = true;

    config.token_status_callback = tokenStatusCallback;

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    // Serial.println("Waiting for Firebase Authentication...");
    // while (!Firebase.isInitialized()) {
    //     vTaskDelay(pdMS_TO_TICKS(100));
    // }
    // Serial.println("Firebase ready and authenticated.");
}
