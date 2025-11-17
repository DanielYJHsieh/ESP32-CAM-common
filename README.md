# ESP32-CAM-common
ESP32-CAM 常用專案集合

## 📋 專案列表

### 1. LED 閃爍入門專案 (led_blink_starter)
- **位置**: `led_blink_starter/`
- **框架**: Arduino
- **說明**: ESP32-CAM 的入門級專案，控制板載 LED 進行閃爍
- **適合對象**: 初學者
- **硬體**: ESP32-CAM (ESP32-S)
- **功能**:
  - 控制閃光燈 LED (GPIO 4)
  - 控制板載紅色 LED (GPIO 33)
  - 串口輸出狀態訊息
  - 進階 PWM 控制和特效

### 2. ESP-IDF LED 測試專案 (esp32-cam_led_test)
- **位置**: `esp32-cam_led_test/`
- **框架**: ESP-IDF 5.5
- **說明**: 使用 ESP-IDF 框架的 LED 閃爍測試專案
- **適合對象**: 進階使用者
- **硬體**: ESP32-CAM
- **功能**:
  - LED 每秒閃爍
  - FreeRTOS 任務管理
  - ESP_LOG 日誌輸出
  - GPIO 4 和 GPIO 33 控制

### 3. HTTP MJPEG 串流專案 (esp32-cam_http_stream) ⭐ NEW
- **位置**: `esp32-cam_http_stream/`
- **框架**: ESP-IDF 5.5
- **說明**: 即時視訊串流伺服器 (方案 B: 直接串流)
- **適合對象**: 進階使用者
- **硬體**: ESP32-CAM + OV2640
- **功能**:
  - HTTP MJPEG 即時串流 (按需啟動，節省流量)
  - Web UI 控制介面 (Stream/Stop/Capture)
  - 單張拍照功能 (自動清除緩存，確保最新畫面)
  - 本地網域限制 (同一子網路才能訪問)
  - 可調解析度 (QVGA ~ UXGA)
  - 三緩衝優化性能 (384KB × 3 in PSRAM)

## 🔌 UART 接線指南

請參閱 [UART_WIRING_GUIDE.md](UART_WIRING_GUIDE.md) 了解如何使用 USB 轉 TTL 模組連接和上傳程式到 ESP32-CAM。

### 快速接線

```
USB 轉 TTL → ESP32-CAM
5V      → 5V
GND     → GND
TX      → U0R (RX)
RX      → U0T (TX)
```

**上傳程式**: 需要將 GPIO 0 短接到 GND

## 🚀 快速開始

### Arduino 專案 (led_blink_starter)

1. **克隆專案**
   ```bash
   git clone https://github.com/DanielYJHsieh/ESP32-CAM-common.git
   ```

2. **準備硬體**
   - ESP32-CAM 開發板
   - USB 轉 TTL 模組 (FTDI/CH340/CP2102)
   - 杜邦線

3. **開啟專案**
   - 使用 Arduino IDE 開啟 `led_blink_starter/led_blink_starter.ino`

4. **設定開發板**
   - 工具 → 開發板 → AI Thinker ESP32-CAM
   - 工具 → 連接埠 → 選擇對應 COM 埠

5. **上傳程式**
   - 短接 GPIO 0 到 GND
   - 點擊上傳按鈕
   - 上傳完成後斷開短接，按 RESET

### ESP-IDF 專案 (esp32-cam_led_test)

1. **設定 ESP-IDF 環境**
   ```powershell
   # Windows PowerShell
   . $env:IDF_PATH\export.ps1
   ```

2. **切換到專案目錄**
   ```bash
   cd esp32-cam_led_test
   ```

3. **編譯專案**
   ```bash
   idf.py build
   ```

4. **上傳到裝置**
   ```bash
   idf.py -p COM3 flash
   ```

5. **監看輸出**
   ```bash
   idf.py -p COM3 monitor
   ```
   按 `Ctrl+]` 退出監視器

## 📚 學習資源

- [ESP32-CAM 官方文檔](https://github.com/espressif/esp32-camera)
- [Arduino ESP32](https://github.com/espressif/arduino-esp32)
- [ESP32 接腳參考](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)

## 🛠️ 開發環境

### Arduino IDE 專案
- **Arduino IDE** 1.8.x 或更高版本
- **ESP32 Board Support** (透過開發板管理員安裝)
- **驅動程式**: CH340 或 CP2102 (依你的 USB 轉 TTL 模組而定)

### ESP-IDF 專案
- **ESP-IDF** v4.4 或更高版本 (測試版本: v5.5.1)
- **Python** 3.7+
- **CMake** 3.16+
- **Ninja** 建置工具

## 📝 專案狀態

- ✅ LED 閃爍入門專案 (Arduino)
- ✅ ESP-IDF LED 測試專案
- 🔄 更多專案陸續加入...

## 📄 授權

本專案採用 MIT 授權條款。

---

**Happy Coding! 🎉**
