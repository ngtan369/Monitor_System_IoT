#ifndef GLOBALS_H
#define GLOBALS_H

// include libraries
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>  // ---
#include <DHT20.h>
#include <ESPAsyncWebServer.h>  //--
#include <Firebase_ESP_Client.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <Update.h>
#include <WiFi.h>
#include <Wire.h>
#include <stdint.h>

#include "../env.h"
#include "../src/connect/Firebase.h"
#include "../src/connect/TaskWebserver.h"
#include "../src/connect/TaskWifi.h"
#include "../src/device/TaskDHT20.h"
#include "../src/device/TaskFan.h"
#include "../src/device/TaskLed.h"

#define SDA 11
#define SCL 12
#define soil_pin 1
#define light_sensor_pin 2
#define led_pin 6    // D3
#define relay_pin 8  // D5
#define fan_pin 10   // D7
#define echo 21      // D10
#define trig 18      // D8
#define BOOT_BUTTON 0
#define LED_ONBOARD 48               // D13
#define FLASH_DELAY_NO_WIFI 3000     
#define FLASH_DELAY_AP_MODE 200
#define FLASH_DELAY_NO_INTERNET 1000
#endif