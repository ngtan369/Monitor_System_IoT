#include "sensor.h"

DHT20 dht20;
float temp;
float humi;
uint8_t light = 0;
QueueHandle_t dataQueue = nullptr; 

void TaskSensors(void* pvParameters) {
    while (true) {
        if (dht20.read() == DHT20_OK) {
            temp = dht20.getTemperature();
            humi = dht20.getHumidity();
        } else {
            Serial.println("DHT Error");
        }
        light = analogRead(LIGHT_SENSOR_PIN);
        SensorData data;
        data.temp = temp;
        data.humi = humi;
        data.light = light;
        if (dataQueue != NULL) {
            xQueueSend(dataQueue, &data, 0);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void initSensor() {
    Wire.begin(SDA, SCL);
    dht20.begin();
    pinMode(LIGHT_SENSOR_PIN, INPUT);
    dataQueue = xQueueCreate(10, sizeof(SensorData));
    xTaskCreate(
        TaskSensors,
        "TaskSensors",
        4096,
        nullptr,
        1,
        nullptr
    );
}