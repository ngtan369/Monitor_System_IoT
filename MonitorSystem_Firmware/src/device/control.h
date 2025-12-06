#ifndef TASK_LED_H
#define TASK_LED_H

#include <Adafruit_NeoPixel.h>

#define LED_PIN 6    // D3
#define FAN_PIN 10   // D7
#define LED_ONBOARD 48 // 2 old onboard LED

extern TaskHandle_t xTaskLedHandle;

struct ControlData {
    int fan;
    int led;
};

extern QueueHandle_t controlQueue;

void initControl();
void led_off();
void led_on();
void Led_Indicate_Task(void*);
#endif