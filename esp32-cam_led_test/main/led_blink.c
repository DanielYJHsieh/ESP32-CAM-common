/*
 * ESP32-CAM LED Blink 測試 (ESP-IDF)
 * 
 * 硬體: ESP32-CAM
 * 功能: LED 每秒閃爍
 * GPIO 4  - 閃光燈 LED
 * GPIO 33 - 板載紅色 LED
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

// LED 腳位定義
#define LED_FLASH_GPIO    4    // 閃光燈 LED
#define LED_BUILTIN_GPIO  33   // 板載紅色 LED

// 閃爍間隔 (毫秒)
#define BLINK_PERIOD_MS   1000

static const char *TAG = "LED_BLINK";

void app_main(void)
{
    ESP_LOGI(TAG, "===== ESP32-CAM LED Blink 測試 =====");
    ESP_LOGI(TAG, "使用 ESP-IDF 框架");
    ESP_LOGI(TAG, "閃爍間隔: %d ms", BLINK_PERIOD_MS);
    
    // 配置 GPIO 為輸出模式
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << LED_FLASH_GPIO) | (1ULL << LED_BUILTIN_GPIO),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&io_conf);
    
    ESP_LOGI(TAG, "GPIO 初始化完成");
    ESP_LOGI(TAG, "開始 LED 閃爍循環...\n");
    
    uint32_t blink_count = 0;
    bool led_state = false;
    
    while (1) {
        // 切換 LED 狀態
        led_state = !led_state;
        
        // 設定兩個 LED
        gpio_set_level(LED_FLASH_GPIO, led_state);
        gpio_set_level(LED_BUILTIN_GPIO, led_state);
        
        // 輸出狀態
        ESP_LOGI(TAG, "LED 狀態: %s (次數: %lu)", 
                 led_state ? "ON " : "OFF", blink_count++);
        
        // 延遲
        vTaskDelay(BLINK_PERIOD_MS / portTICK_PERIOD_MS);
    }
}
