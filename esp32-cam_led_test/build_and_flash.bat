@echo off
REM ESP32-CAM LED Blink 快速編譯上傳腳本 (Windows)

echo =====================================
echo ESP32-CAM LED Blink 自動建置工具
echo =====================================
echo.

REM 檢查 ESP-IDF 環境
if "%IDF_PATH%"=="" (
    echo 錯誤: ESP-IDF 環境未設定
    echo 請先執行: %IDF_PATH%\export.bat
    exit /b 1
)

echo ESP-IDF 路徑: %IDF_PATH%
echo.

REM 設定目標晶片
echo [1/3] 設定目標晶片...
idf.py set-target esp32
if errorlevel 1 (
    echo 錯誤: 設定失敗
    exit /b 1
)

REM 編譯
echo.
echo [2/3] 編譯專案...
idf.py build
if errorlevel 1 (
    echo 錯誤: 編譯失敗
    exit /b 1
)

REM 上傳
echo.
echo [3/3] 上傳到 COM3...
echo 請確認 GPIO 0 已接到 GND
pause

idf.py -p COM3 flash monitor

echo.
echo 完成!
