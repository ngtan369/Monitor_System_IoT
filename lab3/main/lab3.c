#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
const char *pcTextForTask1 = "Task 1 is running";
const char *pcTextForTask2 = "Task 2 is running";

void vTaskContinuous(void *pvParameters) {
  char *pcTaskName = (char *) pvParameters;
  for (;;) {
    printf("%s on Core %d\n", pcTaskName, xPortGetCoreID());
    for(int i=0; i<1000000; i++) { __asm__("nop");} 
  }
//   vTaskDelay(pdMS_TO_TICKS(10));
}
void vTaskPeriodic(void *pvParameters) {
  for (;;) {
    printf("--- High Priority Task Preempting ---\n");
    vTaskDelay(pdMS_TO_TICKS(2000)); // Block for 2000ms
  }
}


void app_main(void) {
    
    // Equal Priority (1)
    xTaskCreatePinnedToCore(vContinuousTask, "Task 1", 2048, "Task 1", 1, NULL, 1);
    xTaskCreatePinnedToCore(vContinuousTask, "Task 2", 2048, "Task 2", 1, NULL, 1);

    // Higher Priority (2)
    xTaskCreatePinnedToCore(vPeriodicTask, "Task 3", 2048, NULL, 2, NULL, 1);
}