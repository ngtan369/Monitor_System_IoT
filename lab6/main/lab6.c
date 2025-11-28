#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_err.h"

#include "nvs_flash.h"
#include "tcpip_adapter.h"

//================= CẤU HÌNH WIFI =================
#define WIFI_SSID      "ESP32_LAB6"
#define WIFI_PASS      "12345678"

//=================================================

static const char *TAG = "LAB06_WIFI";

// Hàm in thông tin AP từ kết quả scan
static void print_ap_info(wifi_ap_record_t *list, uint16_t ap_count)
{
    for (int i = 0; i < ap_count; i++) {
        char *authmode_str;
        switch (list[i].authmode) {
        case WIFI_AUTH_OPEN:
            authmode_str = "OPEN";
            break;
        case WIFI_AUTH_WEP:
            authmode_str = "WEP";
            break;
        case WIFI_AUTH_WPA_PSK:
            authmode_str = "WPA_PSK";
            break;
        case WIFI_AUTH_WPA2_PSK:
            authmode_str = "WPA2_PSK";
            break;
        case WIFI_AUTH_WPA_WPA2_PSK:
            authmode_str = "WPA/WPA2_PSK";
            break;
        default:
            authmode_str = "UNKNOWN";
            break;
        }

        printf("AP %2d: ssid=%s, rssi=%d, auth=%s\n",
               i + 1,
               (char *)list[i].ssid,
               list[i].rssi,
               authmode_str);
    }
}

// Event handler theo đúng tài liệu lab (system_event_t)
static esp_err_t wifi_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {

    case SYSTEM_EVENT_STA_START:
        printf("[EVENT] STA_START -> Bắt đầu scan WiFi...\n");
        // Cấu hình scan
        {
            wifi_scan_config_t scanConf = {
                .ssid = NULL,
                .bssid = NULL,
                .channel = 0,
                .show_hidden = 1
            };
            ESP_ERROR_CHECK(esp_wifi_scan_start(&scanConf, false));
        }
        break;

    case SYSTEM_EVENT_SCAN_DONE: {
        printf("[EVENT] SCAN_DONE: tìm thấy %d AP\n",
               event->event_info.scan_done.number);

        uint16_t ap_count = event->event_info.scan_done.number;
        if (ap_count > 0) {
            wifi_ap_record_t *ap_list =
                (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * ap_count);
            if (ap_list != NULL) {
                ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_list));
                print_ap_info(ap_list, ap_count);
                free(ap_list);
            } else {
                printf("Không đủ RAM để cấp phát danh sách AP!\n");
            }
        }

        printf("[LAB06] Bắt đầu kết nối tới AP: %s\n", WIFI_SSID);
        ESP_ERROR_CHECK(esp_wifi_connect());
        break;
    }

    case SYSTEM_EVENT_STA_CONNECTED:
        printf("[EVENT] STA_CONNECTED: Đã kết nối tới AP.\n");
        break;

    case SYSTEM_EVENT_STA_GOT_IP:
        printf("[EVENT] GOT_IP: IP của ESP32 là " IPSTR "\n",
               IP2STR(&event->event_info.got_ip.ip_info.ip));
        printf("[LAB06] Bây giờ có thể gửi/nhận dữ liệu TCP/UDP.\n");
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        printf("[EVENT] STA_DISCONNECTED: Mất kết nối, thử reconnect...\n");
        ESP_ERROR_CHECK(esp_wifi_connect());
        break;

    default:
        // Bạn có thể in thêm debug nếu muốn
        // printf("[EVENT] id=%d\n", event->event_id);
        break;
    }

    return ESP_OK;
}

void app_main(void)
{
    // Khởi tạo NVS (lưu cấu hình WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Trường hợp NVS lỗi, erase rồi init lại
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Khởi tạo TCP/IP stack
    tcpip_adapter_init();

    // Đăng ký event handler WiFi
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));

    // Khởi tạo WiFi với config mặc định
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Lưu config WiFi trong RAM (không ghi flash mỗi lần)
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    // Chọn mode: Station (STA)
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // Cấu hình SSID & Password cho STA
    wifi_config_t sta_config = {
        .sta = {
            // nhớ không được bỏ dấu "" ở đây
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .bssid_set = 0,
        }
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));

    // Start WiFi → sẽ phát sinh event SYSTEM_EVENT_STA_START
    ESP_ERROR_CHECK(esp_wifi_start());

    printf("[LAB06] WiFi STA đã start, chờ event...\n");

    // Không cần vòng while ở đây, FreeRTOS sẽ chạy scheduler
    // Nếu muốn, có thể thêm 1 task riêng để làm việc sau khi đã có IP.
}

