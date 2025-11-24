#ifndef TASK_LED_H
#define TASK_LED_H

#include "globals.h"
enum LED_COLOR {
    red,
    orange,
    yellow,
    green,
    blue,
    indigo,
    purple,
    white,
    black
};
extern LED_COLOR led_color;
extern volatile bool led_mode;
extern TaskHandle_t xTaskLedHandle;
void initLed();
void led_off();
void led_on();
void Led_Indicate_Task(void*);
#endif