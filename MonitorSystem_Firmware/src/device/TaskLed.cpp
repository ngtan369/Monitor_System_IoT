#include "TaskLed.h"

Adafruit_NeoPixel pixels(4, led_pin, NEO_GRB + NEO_KHZ800);
LED_COLOR led_color;
volatile bool led_mode = 0;  // auto
void TaskLed(void* pvParameters) {
    while (1) {
        switch (led_color) {
            case red:
                pixels.setPixelColor(1, pixels.Color(150, 0, 0));
                pixels.setPixelColor(2, pixels.Color(150, 0, 0));
                break;
            case orange:
                pixels.setPixelColor(1, pixels.Color(150, 80, 0));
                pixels.setPixelColor(2, pixels.Color(150, 80, 0));
                break;
            case yellow:
                pixels.setPixelColor(1, pixels.Color(150, 150, 0));
                pixels.setPixelColor(2, pixels.Color(150, 150, 0));
                break;
            case green:
                pixels.setPixelColor(1, pixels.Color(0, 150, 0));
                pixels.setPixelColor(2, pixels.Color(0, 150, 0));
                break;
            case blue:
                pixels.setPixelColor(1, pixels.Color(0, 0, 150));
                pixels.setPixelColor(2, pixels.Color(0, 0, 150));
                break;
            case indigo:
                pixels.setPixelColor(1, pixels.Color(75, 0, 130));
                pixels.setPixelColor(2, pixels.Color(75, 0, 130));
                break;
            case purple:
                pixels.setPixelColor(1, pixels.Color(128, 0, 128));
                pixels.setPixelColor(2, pixels.Color(128, 0, 128));
                break;
            case white:
                pixels.setPixelColor(1, pixels.Color(150, 150, 150));
                pixels.setPixelColor(2, pixels.Color(150, 150, 150));
                break;
            case black:
                pixels.setPixelColor(1, pixels.Color(0, 0, 0));
                pixels.setPixelColor(2, pixels.Color(0, 0, 0));
                break;
        }
        pixels.show();
        vTaskDelay(pdMS_TO_TICKS(2000));
        if (!led_mode)
            led_color = static_cast<LED_COLOR>((led_color + 1) % 9);
    }
}


TaskHandle_t xTaskLedHandle = NULL;
void Led_Indicate_Task(void* pvParameters) {
    uint32_t ulFlashDelay = 1000;
    uint32_t ulNotifiedValue;

    while(1){

        // CHỜ thông báo (block vĩnh viễn cho đến khi có notify)
        if (xTaskNotifyWait(
                0x00,
                0xFFFFFFFF,
                &ulNotifiedValue,
                0   // CHỜ tới khi nhận tín hiệu
            ) == pdPASS) 
        {
            ulFlashDelay = ulNotifiedValue;
            Serial.printf("New LED delay: %lu ms\n", ulFlashDelay);
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

void initLed() {
    pinMode(LED_BUILTIN, OUTPUT);
    led_off();
    xTaskCreate(
        Led_Indicate_Task,
        "Led_Indicator",
        2048,
        NULL,
        2,  // Priority cao hơn Wifi_reconnect (nếu cần)
        &xTaskLedHandle);
}
void led_off() {
    digitalWrite(LED_BUILTIN, 0);
}
void led_on() {
    digitalWrite(LED_BUILTIN, 1);
}