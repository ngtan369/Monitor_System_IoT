#include "control.h"

Adafruit_NeoPixel pixels(4, LED_PIN, NEO_GRB + NEO_KHZ800);

QueueHandle_t controlQueue = xQueueCreate(5, sizeof(ControlData));
bool fan_state = false;
bool led_state = false;
void TaskControl(void* pvParameters) {
    while (1) {
        if(controlQueue != NULL) {
            ControlData controlData;
            if (xQueueReceive(controlQueue, &controlData, portMAX_DELAY) == pdPASS) {
                Serial.printf("Control Task: Received fan=%.2f, led=%.2f\n", controlData.fan, controlData.led);
                fan_state = (controlData.fan > 0.5);
                digitalWrite(FAN_PIN, fan_state ? HIGH : LOW);
                led_state = (controlData.led > 0.5);
                if (led_state){
                    pixels.setPixelColor(1, pixels.Color(150, 150, 150));
                    pixels.setPixelColor(2, pixels.Color(150, 150, 150));
                    pixels.show();  
                } else {
                    pixels.setPixelColor(1, pixels.Color(0, 0, 0));
                    pixels.setPixelColor(2, pixels.Color(0, 0, 0));
                    pixels.show();
                }
            } else {
                vTaskDelay(pdMS_TO_TICKS(100));
        }
        }
    }
}

TaskHandle_t xTaskLedHandle = NULL;
void Led_Indicate_Task(void* pvParameters) {
    uint32_t ulFlashDelay = 1000;
    uint32_t ulNotifiedValue;

    while(1){
        if (xTaskNotifyWait(
                0x00,
                0xFFFFFFFF,
                &ulNotifiedValue,
                0   // wait for singal
            ) == pdPASS) 
        {
            ulFlashDelay = ulNotifiedValue;
            // Serial.printf("New LED delay: %lu ms\n", ulFlashDelay);
        }
        if (ulFlashDelay == 0) {
            led_off();
            vTaskDelay(pdMS_TO_TICKS(3000));
            continue;
        }   
        led_on();
        vTaskDelay(pdMS_TO_TICKS(ulFlashDelay / 2));
        led_off();
        vTaskDelay(pdMS_TO_TICKS(ulFlashDelay / 2));
    }
}

void initControl() {
    pinMode(FAN_PIN, OUTPUT);
    pinMode(LED_ONBOARD, OUTPUT); // LED_BUILTIN
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(FAN_PIN, LOW);
    digitalWrite(LED_ONBOARD, LOW);
    digitalWrite(LED_PIN, LOW);

    xTaskCreate(
        Led_Indicate_Task,
        "Led_Indicator",
        2048,
        NULL,
        1,
        &xTaskLedHandle);
    
    xTaskCreate(
        TaskControl,
        "TaskControl",
        4096,
        NULL,
        1,
        NULL);
}

void led_off() {
    digitalWrite(LED_ONBOARD, 0);
}
void led_on() {
    digitalWrite(LED_ONBOARD, 1);
}