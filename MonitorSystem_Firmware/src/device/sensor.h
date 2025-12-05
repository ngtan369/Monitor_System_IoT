#ifndef INC_TASKDHT20_H_
#define INC_TASKDHT20_H_

#include "globals.h"
#include <DHT20.h>
#include <Wire.h>
#define SDA 11
#define SCL 12
#define LIGHT_SENSOR_PIN 2

struct SensorData {
    float temp;
    float humi;
    float light;
};
extern QueueHandle_t dataQueue;

void initSensor();

#endif /* INC_TASKDHT20_H_ */