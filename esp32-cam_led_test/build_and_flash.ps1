# ESP32-CAM LED Blink 自動編譯和上傳腳本
# 使用方法: .\build_and_flash.ps1

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "ESP32-CAM LED Blink 自動建置工具" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

# 檢查是否在正確的目錄
if (-Not (Test-Path "CMakeLists.txt")) {
    Write-Host "錯誤: 請在專案根目錄執行此腳本" -ForegroundColor Red
    exit 1
}

# 設定變數
$PORT = "COM3"
$PROJECT_DIR = Get-Location

Write-Host "[1/4] 檢查 ESP-IDF 環境..." -ForegroundColor Yellow

# 檢查 IDF_PATH 環境變數
if (-Not $env:IDF_PATH) {
    Write-Host "警告: ESP-IDF 環境未設定，嘗試自動設定..." -ForegroundColor Yellow
    
    # 常見的 ESP-IDF 安裝路徑
    $idf_paths = @(
        "$env:USERPROFILE\esp\esp-idf",
        "$env:USERPROFILE\.espressif\esp-idf",
        "C:\Espressif\frameworks\esp-idf-v*",
        "$env:IDF_PATH"
    )
    
    $idf_found = $false
    foreach ($path in $idf_paths) {
        $resolved = Resolve-Path $path -ErrorAction SilentlyContinue
        if ($resolved -and (Test-Path "$resolved\export.ps1")) {
            Write-Host "找到 ESP-IDF: $resolved" -ForegroundColor Green
            & "$resolved\export.ps1"
            $idf_found = $true
            break
        }
    }
    
    if (-Not $idf_found) {
        Write-Host "錯誤: 找不到 ESP-IDF 環境" -ForegroundColor Red
        Write-Host "請先執行: . `$env:IDF_PATH\export.ps1" -ForegroundColor Yellow
        exit 1
    }
}

Write-Host "ESP-IDF 環境: $env:IDF_PATH" -ForegroundColor Green
Write-Host ""

# 檢查 idf.py 是否可用
Write-Host "[2/4] 檢查建置工具..." -ForegroundColor Yellow
$idf_cmd = Get-Command "idf.py" -ErrorAction SilentlyContinue
if (-Not $idf_cmd) {
    Write-Host "錯誤: idf.py 命令不可用" -ForegroundColor Red
    Write-Host "請確認 ESP-IDF 環境已正確設定" -ForegroundColor Yellow
    exit 1
}
Write-Host "建置工具就緒" -ForegroundColor Green
Write-Host ""

# 設定目標晶片
Write-Host "[3/4] 設定目標晶片為 ESP32..." -ForegroundColor Yellow
idf.py set-target esp32
if ($LASTEXITCODE -ne 0) {
    Write-Host "錯誤: 設定目標晶片失敗" -ForegroundColor Red
    exit 1
}
Write-Host ""

# 編譯專案
Write-Host "[4/4] 開始編譯專案..." -ForegroundColor Yellow
Write-Host "這可能需要幾分鐘時間..." -ForegroundColor Cyan
Write-Host ""

idf.py build
if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "錯誤: 編譯失敗" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "編譯成功!" -ForegroundColor Green
Write-Host ""

# 上傳到裝置
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "準備上傳到裝置 (COM3)" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "請確認:" -ForegroundColor Yellow
Write-Host "  1. GPIO 0 已短接到 GND" -ForegroundColor Yellow
Write-Host "  2. ESP32-CAM 已連接到 $PORT" -ForegroundColor Yellow
Write-Host "  3. 已按下 RESET 按鈕" -ForegroundColor Yellow
Write-Host ""

$response = Read-Host "是否繼續上傳? (Y/N)"
if ($response -ne "Y" -and $response -ne "y") {
    Write-Host "已取消上傳" -ForegroundColor Yellow
    exit 0
}

Write-Host ""
Write-Host "開始上傳..." -ForegroundColor Yellow
Write-Host ""

idf.py -p $PORT flash
if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "錯誤: 上傳失敗" -ForegroundColor Red
    Write-Host ""
    Write-Host "請檢查:" -ForegroundColor Yellow
    Write-Host "  - GPIO 0 是否已接到 GND" -ForegroundColor Yellow
    Write-Host "  - COM 埠是否正確" -ForegroundColor Yellow
    Write-Host "  - USB 轉 TTL 驅動是否已安裝" -ForegroundColor Yellow
    exit 1
}

Write-Host ""
Write-Host "上傳成功!" -ForegroundColor Green
Write-Host ""
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "接下來的步驟:" -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "  1. 移除 GPIO 0 和 GND 的短接" -ForegroundColor White
Write-Host "  2. 按下 RESET 按鈕" -ForegroundColor White
Write-Host "  3. LED 將開始閃爍" -ForegroundColor White
Write-Host ""

$monitor = Read-Host "是否開啟串口監視器? (Y/N)"
if ($monitor -eq "Y" -or $monitor -eq "y") {
    Write-Host ""
    Write-Host "啟動串口監視器..." -ForegroundColor Yellow
    Write-Host "按 Ctrl+] 退出" -ForegroundColor Cyan
    Write-Host ""
    Start-Sleep -Seconds 2
    idf.py -p $PORT monitor
}

Write-Host ""
Write-Host "完成!" -ForegroundColor Green
