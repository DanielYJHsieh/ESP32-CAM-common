# ESP32-CAM HTTP MJPEG Stream Server

<div align="center">

![ESP32-CAM](https://img.shields.io/badge/Hardware-ESP32--CAM-blue)
![Framework](https://img.shields.io/badge/Framework-ESP--IDF-green)
![Status](https://img.shields.io/badge/Status-方案B-orange)

</div>

## 📖 專案說明

這是一個基於 ESP-IDF 的 ESP32-CAM HTTP MJPEG 串流伺服器專案。實現**方案 B: 直接串流**架構，ESP32-CAM 直接提供 HTTP 串流給 PC 瀏覽器。

### 架構圖

```
┌─────────────┐    WiFi/HTTP    ┌──────────┐
│  ESP32-CAM  │ ──────────────> │  PC/手機 │
│  HTTP Server│   MJPEG Stream  │  瀏覽器  │
└─────────────┘                 └──────────┘
```

## ✨ 功能特色

- ✅ **即時 MJPEG 串流**: 低延遲視訊串流
- ✅ **Web UI 介面**: 美觀的網頁控制介面
- ✅ **多客戶端支援**: 支援多個瀏覽器同時觀看
- ✅ **單張拍照**: 支援抓拍單張圖片
- ✅ **狀態監控**: 即時顯示相機狀態
- ✅ **可調解析度**: 支援 QVGA ~ UXGA
- ✅ **優化性能**: 使用雙緩衝提升幀率

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

### 解析度選項

| 解析度 | 尺寸 | FPS | 建議用途 |
|--------|------|-----|---------|
| QVGA | 320x240 | ~25 | 快速預覽 |
| VGA | 640x480 | ~15 | **預設/平衡** |
| SVGA | 800x600 | ~10 | 高品質 |
| XGA | 1024x768 | ~5 | 靜態拍照 |
| SXGA | 1280x1024 | ~3 | 最高解析度 |

### 記憶體需求

- **PSRAM**: 必須啟用 (4MB)
- **程式大小**: ~800KB
- **運行記憶體**: ~200KB

## ⚙️ 進階設定

### 修改解析度

編輯 `main/camera_httpd.c`:

```c
.frame_size = FRAMESIZE_VGA,    // 改為 FRAMESIZE_SVGA
```

### 調整 JPEG 品質

```c
.jpeg_quality = 12,    // 0-63, 數字越小品質越高
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
