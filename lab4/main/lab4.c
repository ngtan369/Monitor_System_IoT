#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"

#define LOG_TAG "APP"
#define QUEUE_SIZE 10

typedef enum {
    Humidity = 0,
    Temp = 1,
    Light = 2
} ID_t;

typedef struct {
    ID_t eDataID;
    int32_t lDataValue;
} Data_t;

QueueHandle_t potQueue = NULL;

void handleHumiditySensor(const int32_t value) {
    ESP_LOGI(LOG_TAG, "Humidity: %ld %%", value);
}

void handleTempSensor(const int32_t value) {
    ESP_LOGI(LOG_TAG, "Temp: %ld C", value);
}

void handleLightSensor(const int32_t value) {
    ESP_LOGI(LOG_TAG, "Light: %ld", value);
}

void receptionTask(void * pvParameters) {
    Data_t dataToSend;
    float humidity = 55.5; // hardcode giá trị
    float temp = 23.4;     // hardcode giá trị
    int light = 1234;      // hardcode giá trị
    for (;;) {
        // Giả lập đọc cảm biến DHT
        dataToSend.eDataID = Humidity;
        dataToSend.lDataValue = (int32_t) humidity;
        xQueueSend(potQueue, &dataToSend, pdMS_TO_TICKS(10));

        dataToSend.eDataID = Temp;
        dataToSend.lDataValue = (int32_t) temp;
        xQueueSend(potQueue, &dataToSend, pdMS_TO_TICKS(10));

        // Giả lập đọc cảm biến ánh sáng
        dataToSend.eDataID = Light;
        dataToSend.lDataValue = (int32_t) light;
        xQueueSend(potQueue, &dataToSend, pdMS_TO_TICKS(10));

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void mainHandle(void * pvParameters) {
    Data_t receivedData;
    
    for (;;) {
        if (xQueueReceive(potQueue, &receivedData, portMAX_DELAY) == pdPASS) {
            switch (receivedData.eDataID) {
                case Humidity: 
                    handleHumiditySensor(receivedData.lDataValue);
                    break;
                case Temp:    
                    handleTempSensor(receivedData.lDataValue);
                    break;
                case Light:   
                    handleLightSensor(receivedData.lDataValue);
                    break;
                default:
                    ESP_LOGE(LOG_TAG, "Error: Invalid ID");
                    break;
            }
        } 
    }
}

void app_main(void) {
    esp_log_level_set(LOG_TAG, ESP_LOG_INFO);
    ESP_LOGI(LOG_TAG, "System Starting...");

    potQueue = xQueueCreate(QUEUE_SIZE, sizeof(Data_t));
    if (potQueue == NULL) {
        ESP_LOGE(LOG_TAG, "Failed to create queue!");
        return;
    }

    xTaskCreate(mainHandle, "mainHandle", 4096, NULL, 5, NULL);
    xTaskCreate(receptionTask, "receptionTask", 4096, NULL, 5, NULL);
}