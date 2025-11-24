#include "TaskLight.h"

uint8_t light = 0;

void TaskLight(void* pvParameters) {
    while (1) {
        light = (analogRead(light_sensor_pin) / 4095.0) * 100;
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void initLight() {
    pinMode(light_sensor_pin, INPUT);
    xTaskCreate(TaskLight, "TaskLight", 2048, NULL, 1, NULL);
}