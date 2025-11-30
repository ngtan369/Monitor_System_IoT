#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

int msg1_count = 1;
int msg2_count = 1;

void msg1_cb(TimerHandle_t xTimer){
    printf("%d: ahihi\n", msg1_count);
    msg1_count++;
    if (msg1_count > 10) {
        xTimerStop(xTimer, 0);
    }
}

void msg2_cb(TimerHandle_t xTimer){
    printf("%d: ihaha\n", msg2_count);
    msg2_count++;
    if (msg2_count > 5) {
        xTimerStop(xTimer, 0);
    }
}

void app_main(void)
{
    TimerHandle_t timer1 = xTimerCreate("msg1", pdMS_TO_TICKS(2000), pdTRUE, NULL, msg1_cb);
    TimerHandle_t timer2 = xTimerCreate("msg2", pdMS_TO_TICKS(3000), pdTRUE, NULL, msg2_cb);
    xTimerStart(timer1, 0);
    xTimerStart(timer2, 0);
}
