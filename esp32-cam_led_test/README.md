# ESP32-CAM LED Blink 測試 (ESP-IDF)

## 專案說明

這是使用 ESP-IDF 框架開發的 ESP32-CAM LED 閃爍測試專案。

## 硬體配置

- **開發板**: ESP32-CAM
- **LED**: 
  - GPIO 4 - 閃光燈 LED
  - GPIO 33 - 板載紅色 LED
- **連接埠**: COM3

## 功能

- LED 每秒閃爍一次
- 使用 FreeRTOS 任務管理
- ESP_LOG 輸出狀態訊息

## 編譯和上傳

### 方法 1: 使用 PowerShell 腳本 (自動化)

```powershell
.\build_and_flash.ps1
```

### 方法 2: 手動命令

```bash
# 設定目標晶片
idf.py set-target esp32

# 編譯專案
idf.py build

# 上傳到裝置 (COM3)
idf.py -p COM3 flash

# 監看串口輸出
idf.py -p COM3 monitor
```

### 方法 3: 一鍵編譯上傳並監看

```bash
idf.py -p COM3 flash monitor
```

## 系統需求

- ESP-IDF v4.4 或更高版本
- Python 3.7+
- CMake 3.16+

## 輸出範例

```
I (320) LED_BLINK: ===== ESP32-CAM LED Blink 測試 =====
I (330) LED_BLINK: 使用 ESP-IDF 框架
I (330) LED_BLINK: 閃爍間隔: 1000 ms
I (340) LED_BLINK: GPIO 初始化完成
I (340) LED_BLINK: 開始 LED 閃爍循環...

I (340) LED_BLINK: LED 狀態: ON  (次數: 0)
I (1340) LED_BLINK: LED 狀態: OFF (次數: 1)
I (2340) LED_BLINK: LED 狀態: ON  (次數: 2)
```

## 進入下載模式

上傳程式前需要讓 ESP32-CAM 進入下載模式：

1. 短接 GPIO 0 到 GND
2. 按下 RESET 或重新上電
3. 開始上傳
4. 上傳完成後移除短接
5. 按 RESET 執行程式

## 故障排除

### 找不到串口

確認 USB 轉 TTL 模組已連接並安裝驅動。

### 上傳失敗

1. 確認 GPIO 0 已接到 GND
2. 檢查 COM 埠是否正確
3. 嘗試按下 RESET 後立即上傳

### 編譯錯誤

確認 ESP-IDF 環境已正確設定：

```bash
. $HOME/esp/esp-idf/export.sh
```

Windows PowerShell:
```powershell
. $env:IDF_PATH\export.ps1
```
