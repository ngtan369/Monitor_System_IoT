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

// Handle của Task LED, cần thiết để các Task khác gửi Notification
TaskHandle_t xTaskLedHandle = NULL;

void Led_Indicate_Task(void* pvParameters) {
    // Giá trị mặc định ban đầu: NO_WIFI (1000ms)
    uint32_t ulFlashDelay = 1000;
    uint32_t ulNotifiedValue;
    const TickType_t xMaxBlockTime = portMAX_DELAY;  // Chờ vô thời hạn

    for (;;) {
        // 1. Chờ nhận Notification:
        // Nếu có thông báo, Task sẽ nhận giá trị và ulNotifiedValue sẽ được cập nhật.
        // Nếu không, Task sẽ hết thời gian chờ và tiếp tục với ulFlashDelay hiện tại.
        // Ở đây ta dùng xTaskNotifyWait để chờ.
        if (xTaskNotifyWait(
                0x00,              // Clear bits on entry (không dùng)
                0xFFFFFFFF,        // Clear all bits on exit (luôn xóa notification)
                &ulNotifiedValue,  // Nơi lưu trữ giá trị nhận được
                0) == pdPASS) {    // Không cần block, chỉ kiểm tra nhanh (0 ticks)

            // Nếu nhận được thông báo, cập nhật chu kỳ delay mới
            // Giá trị gửi trong Notification là chu kỳ ms
            ulFlashDelay = ulNotifiedValue;
            Serial.printf("LED Task: New flash delay set to %lu ms\n", ulFlashDelay);
        }

        // 2. Thực hiện nháy LED với delay hiện tại
        led_on();
        vTaskDelay(pdMS_TO_TICKS(ulFlashDelay / 2));  // Chia 2 vì nháy LED_ON/LED_OFF

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