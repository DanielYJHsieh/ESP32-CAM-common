# ESP32-CAM HTTP MJPEG Stream Server

<div align="center">

![ESP32-CAM](https://img.shields.io/badge/Hardware-ESP32--CAM-blue)
![Framework](https://img.shields.io/badge/Framework-ESP--IDF-green)
![PSRAM](https://img.shields.io/badge/PSRAM-4MB-red)
![Status](https://img.shields.io/badge/Status-Production-brightgreen)

</div>

## 📖 專案說明

這是一個基於 ESP-IDF 的 ESP32-CAM HTTP MJPEG 串流伺服器專案。實現**方案 B: 直接串流**架構，ESP32-CAM 直接提供 HTTP 串流給 PC 瀏覽器。

### 🎯 性能規格

- **解析度**: **UXGA (1600x1200)** - OV2640 最大解析度
- **幀率**: 約 **8-12 FPS** @ UXGA
- **緩衝**: **三緩衝** (3 × 384KB in PSRAM)
- **PSRAM**: **4MB** 完全啟用
- **JPEG 品質**: 12 (平衡品質與速度)
- **XCLK**: 20MHz (穩定最高頻率)

### 架構圖

```
┌─────────────┐    WiFi/HTTP    ┌──────────┐
│  ESP32-CAM  │ ──────────────> │  PC/手機 │
│  HTTP Server│   MJPEG Stream  │  瀏覽器  │
│   + PSRAM   │   UXGA 1600x1200│          │
└─────────────┘                 └──────────┘
```

## ✨ 功能特色

- ✅ **即時 MJPEG 串流**: 低延遲視訊串流
- ✅ **UXGA 解析度**: 1600x1200 最高畫質
- ✅ **PSRAM 加速**: 4MB PSRAM 三緩衝技術
- ✅ **Web UI 介面**: 美觀的網頁控制介面
- ✅ **多客戶端支援**: 支援多個瀏覽器同時觀看
- ✅ **單張拍照**: 支援抓拍單張圖片
- ✅ **狀態監控**: 即時顯示相機狀態和 PSRAM 診斷
- ✅ **優化性能**: 三緩衝提升流暢度

## 🔧 硬體需求

- **開發板**: ESP32-CAM (AI-Thinker)
- **相機模組**: OV2640
- **USB 轉 TTL**: 用於編程 (FTDI/CH340/CP2102)
- **電源**: 5V/2A (建議使用外部電源)

## 📋 接腳定義

```
ESP32-CAM (AI-Thinker) Pin Mapping:
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
PWDN    GPIO 32
RESET   -1 (軟體重置)
XCLK    GPIO 0
SIOD    GPIO 26 (I2C SDA)
SIOC    GPIO 27 (I2C SCL)

D7      GPIO 35
D6      GPIO 34
D5      GPIO 39
D4      GPIO 36
D3      GPIO 21
D2      GPIO 19
D1      GPIO 18
D0      GPIO 5
VSYNC   GPIO 25
HREF    GPIO 23
PCLK    GPIO 22
```

## 🚀 快速開始

### 1. 安裝 ESP-IDF

確保已安裝 ESP-IDF v4.4 或更高版本。

```powershell
# Windows - 設定環境
. C:\Users\<YOUR_USER>\esp\v5.5.1\esp-idf\export.ps1
```

### 2. 配置 WiFi

編輯 `sdkconfig.defaults` 或使用 menuconfig:

```bash
idf.py menuconfig
```

設定路徑: `Component config` → `WiFi Configuration`

```
WiFi SSID: YOUR_WIFI_SSID
WiFi Password: YOUR_WIFI_PASSWORD
```

### 3. 添加相機驅動

```bash
idf.py add-dependency "espressif/esp32-camera"
```

### 4. 編譯專案

```bash
cd esp32-cam_http_stream
idf.py build
```

### 5. 上傳到裝置

```bash
# 短接 GPIO 0 到 GND 進入下載模式
idf.py -p COM3 flash
```

### 6. 監看輸出

```bash
idf.py -p COM3 monitor
```

按 `Ctrl+]` 退出監視器。

## 🌐 使用方式

### 1. 取得 IP 位址

從串口監視器找到 ESP32-CAM 的 IP 位址:

```
I (5234) camera_httpd: Camera stream server ready!
I (5234) camera_httpd: Open http://192.168.1.100/ in your browser
```

### 2. 開啟瀏覽器

在電腦或手機瀏覽器輸入:

```
http://192.168.1.100/
```

### 3. 可用的 URL

| URL | 功能 | 說明 |
|-----|------|------|
| `/` | 主頁 | Web UI 控制介面 |
| `/stream` | 串流 | MJPEG 即時串流 |
| `/capture` | 拍照 | 單張 JPEG 圖片 |
| `/status` | 狀態 | JSON 格式相機狀態 |

## 📊 性能參數

### 當前配置 (最佳化)

- **解析度**: UXGA (1600×1200)
- **幀率**: 8-12 FPS
- **JPEG 品質**: 12
- **幀緩衝**: 3 個 (三緩衝)
- **緩衝大小**: 384KB × 3 = 1152KB
- **PSRAM 使用**: ~1.2MB / 4MB
- **XCLK 頻率**: 20MHz

### 解析度選項

| 解析度 | 尺寸 | FPS | 緩衝需求 | 建議用途 |
|--------|------|-----|---------|---------|
| QVGA | 320×240 | ~30 | ~24KB | 快速預覽/低頻寬 |
| VGA | 640×480 | ~25 | ~60KB | 平衡選項 |
| SVGA | 800×600 | ~20 | ~96KB | 高品質串流 |
| XGA | 1024×768 | ~15 | ~150KB | 高解析度 |
| SXGA | 1280×1024 | ~10 | ~250KB | 超高解析度 |
| **UXGA** | **1600×1200** | **~10** | **~384KB** | **最大解析度** ⭐ |

### 記憶體需求

- **PSRAM**: **必須啟用** (4MB)
  - 實際可用: ~4MB
  - 三緩衝使用: ~1.2MB
  - 剩餘可用: ~2.8MB
- **程式大小**: ~993KB (0xf27f0 bytes)
- **Flash 佔用**: 24% (76% 剩餘)
- **運行記憶體**: ~300KB DRAM

### PSRAM 診斷輸出範例

```
I (6723) camera_httpd: === PSRAM Diagnostic ===
I (6723) camera_httpd: PSRAM Total: 4180912 bytes (3.99 MB)
I (6723) camera_httpd: PSRAM Free: 4178516 bytes (3.98 MB)
I (6733) camera_httpd: PSRAM Used: 2396 bytes (0.00 MB)
I (6733) camera_httpd: PSRAM Largest Free Block: 4128768 bytes (3.94 MB)
I (6743) camera_httpd: Testing PSRAM allocation...
I (6743) camera_httpd: ✓ PSRAM test allocation successful (1KB)
I (6753) camera_httpd: ✓ PSRAM allocation successful (16KB)
I (6753) camera_httpd: ✓ PSRAM large allocation successful (64KB)
```

## ⚙️ 進階設定

### 修改解析度

編輯 `main/camera_httpd.c`:

```c
.frame_size = FRAMESIZE_UXGA,    // 當前: UXGA (1600x1200)
                                 // 可選: FRAMESIZE_SVGA, FRAMESIZE_VGA 等
```

### 調整 JPEG 品質

```c
.jpeg_quality = 12,    // 0-63, 數字越小品質越高
                       // 12 = 平衡品質與速度 (推薦)
                       // 10 = 更高品質，較慢
                       // 15 = 較低品質，更快
```

### 調整幀緩衝數量

```c
.fb_count = 3,         // 當前: 3 (三緩衝，最流暢)
                       // 2 = 雙緩衝 (節省記憶體)
                       // 1 = 單緩衝 (最小記憶體)
```

### PSRAM 配置 (sdkconfig.defaults)

```ini
# PSRAM Configuration - AI-Thinker ESP32-CAM (4MB PSRAM)
CONFIG_ESP32_SPIRAM_SUPPORT=y
CONFIG_SPIRAM=y
CONFIG_SPIRAM_BOOT_INIT=y
CONFIG_SPIRAM_USE_MALLOC=y
CONFIG_SPIRAM_TYPE_AUTO=y
CONFIG_SPIRAM_SPEED_40M=y
```

### 修改相機參數

在 `init_camera()` 函數中調整:

```c
s->set_brightness(s, 0);     // -2 to 2
s->set_contrast(s, 0);       // -2 to 2
s->set_saturation(s, 0);     // -2 to 2
s->set_hmirror(s, 0);        // 0 = 正常, 1 = 水平鏡像
s->set_vflip(s, 0);          // 0 = 正常, 1 = 垂直翻轉
```

## 🐛 故障排除

### 問題 1: 找不到相機

**症狀**: `Camera Init Failed`

**解決方法**:
- 檢查相機排線連接
- 確認接腳定義正確
- 檢查 PSRAM 是否啟用

### 問題 2: 串流卡頓

**症狀**: 畫面延遲或掉幀

**解決方法**:
- 降低解析度 (改為 QVGA 或 VGA)
- 降低 JPEG 品質 (增加數字至 15-20)
- 確保 WiFi 信號強度
- 減少同時觀看的客戶端數量

### 問題 3: 無法連接 WiFi

**症狀**: WiFi 連接失敗

**解決方法**:
- 確認 WiFi SSID 和密碼正確
- 檢查 WiFi 是 2.4GHz (ESP32 不支援 5GHz)
- 重啟路由器
- 使用 `idf.py monitor` 查看錯誤訊息

### 問題 4: 記憶體不足

**症狀**: 系統崩潰或重啟

**解決方法**:
- 確認 PSRAM 已啟用
- 降低解析度
- 減少 frame buffer 數量 (改為 `fb_count = 1`)

## 📁 專案結構

```
esp32-cam_http_stream/
├── main/
│   ├── camera_httpd.c          # 主程式 (串流伺服器)
│   └── CMakeLists.txt          # 元件配置
├── CMakeLists.txt              # 專案配置
├── sdkconfig.defaults          # 預設配置
├── partitions.csv              # 分區表
├── config.conf                 # 配置檔案
├── .gitignore                  # Git 忽略檔案
└── README.md                   # 專案說明
```

## 🔄 後續擴展 - 方案 A

方案 A 將加入樹莓派作為中央伺服器:

```
┌─────────────┐    WiFi    ┌──────────────┐    網路    ┌──────────┐
│  ESP32-CAM  │ ────────> │  樹莓派      │ <────────> │  手機/PC │
│  (相機端)   │   串流     │  (伺服器)    │   Web UI   │  (客戶端)│
└─────────────┘            └──────────────┘            └──────────┘
```

**樹莓派功能**:
- RTSP/HLS 串流伺服器
- 錄影和回放
- 動態偵測
- 多相機管理
- 遠端訪問

## 📚 參考資料

- [ESP32-Camera 官方驅動](https://github.com/espressif/esp32-camera)
- [ESP-IDF 編程指南](https://docs.espressif.com/projects/esp-idf/)
- [HTTP Server API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/protocols/esp_http_server.html)

## 🤝 貢獻

歡迎提交 Issues 和 Pull Requests！

## 📄 授權

MIT License

---

**開發者**: Daniel YJ Hsieh  
**日期**: 2025-11-03  
**版本**: 1.0.0  
**狀態**: 方案 B 完成 ✅
