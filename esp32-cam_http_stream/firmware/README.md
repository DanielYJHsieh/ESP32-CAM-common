# ESP32-CAM HTTP Stream Firmware

## 韌體檔案說明

本目錄包含已編譯的 ESP32-CAM 韌體檔案，可直接燒錄使用。

### 檔案列表

- `esp32-cam_http_stream.bin` - 主程式韌體
- `bootloader.bin` - Bootloader
- `partition-table.bin` - 分區表

### 燒錄指令

使用 esptool.py 燒錄：

```bash
esptool.py --chip esp32 -p COM3 -b 460800 \
  --before=default_reset --after=hard_reset write_flash \
  --flash_mode dio --flash_freq 40m --flash_size 4MB \
  0x1000 bootloader.bin \
  0x8000 partition-table.bin \
  0x10000 esp32-cam_http_stream.bin
```

或使用專案提供的 PowerShell 腳本：

```powershell
.\build_and_flash.ps1 -Action flash -Port COM3
```

### 功能特性

- **HTTP MJPEG 串流**: 即時影像串流
- **本地網路限制**: 僅允許同網段存取
- **HTTP Basic 認證**: 需帳號密碼登入
- **Web 控制介面**: Stream/Stop/Capture 按鈕控制
- **高解析度**: UXGA (1600×1200) @ 8-12 FPS
- **三重緩衝**: 使用 PSRAM 提升效能

### 預設設定

- **WiFi SSID**: `lulumi_ap`
- **WiFi Password**: `1978120505`
- **HTTP Username**: `hsieh`
- **HTTP Password**: `1395`
- **IP Address**: DHCP 自動分配

### 編譯資訊

- **ESP-IDF 版本**: v5.5.1
- **編譯時間**: 2025-11-18 08:00:17
- **Git Commit**: a7fe59a-dirty
- **硬體**: ESP32-CAM (AI-Thinker)
- **PSRAM**: 4MB

### 注意事項

⚠️ **燒錄前請確認**:
1. GPIO 0 已連接到 GND (進入燒錄模式)
2. 使用正確的 COM Port
3. 電源供應穩定 (建議 5V/2A)

⚠️ **安全警告**:
- WiFi 密碼和 HTTP 認證密碼已包含在韌體中
- 使用前請修改 `sdkconfig.defaults` 並重新編譯
- 不要將包含敏感資訊的韌體公開分享

### 如何修改密碼

編輯 `sdkconfig.defaults` 檔案：

```ini
# WiFi Configuration
CONFIG_ESP_WIFI_SSID="your_ssid"
CONFIG_ESP_WIFI_PASSWORD="your_password"

# HTTP Authentication
CONFIG_HTTP_AUTH_USERNAME="your_username"
CONFIG_HTTP_AUTH_PASSWORD="your_password"
```

然後重新編譯：

```powershell
.\build_and_flash.ps1 -Action all -Port COM3
```
