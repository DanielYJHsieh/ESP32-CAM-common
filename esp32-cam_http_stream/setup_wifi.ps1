# ESP32-CAM HTTP Stream - WiFi Configuration Helper
# This script helps you configure WiFi credentials

Write-Host "==================================" -ForegroundColor Cyan
Write-Host "WiFi Configuration Helper" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""

# Get WiFi credentials from user
$SSID = Read-Host "Enter WiFi SSID"
$Password = Read-Host "Enter WiFi Password" -AsSecureString
$PasswordPlain = [System.Runtime.InteropServices.Marshal]::PtrToStringAuto(
    [System.Runtime.InteropServices.Marshal]::SecureStringToBSTR($Password)
)

Write-Host ""
Write-Host "Configuration:" -ForegroundColor Yellow
Write-Host "  SSID: $SSID"
Write-Host "  Password: $('*' * $PasswordPlain.Length)"
Write-Host ""

$Confirm = Read-Host "Is this correct? (y/n)"
if ($Confirm -ne "y") {
    Write-Host "Cancelled." -ForegroundColor Red
    exit 0
}

# Update sdkconfig.defaults
$ConfigFile = Join-Path $PSScriptRoot "sdkconfig.defaults"
$Content = Get-Content $ConfigFile -Raw

# Replace WiFi SSID
$Content = $Content -replace 'CONFIG_ESP_WIFI_SSID="YOUR_WIFI_SSID"', "CONFIG_ESP_WIFI_SSID=`"$SSID`""
# Replace WiFi Password
$Content = $Content -replace 'CONFIG_ESP_WIFI_PASSWORD="YOUR_WIFI_PASSWORD"', "CONFIG_ESP_WIFI_PASSWORD=`"$PasswordPlain`""

# Save updated configuration
Set-Content -Path $ConfigFile -Value $Content -NoNewline

Write-Host ""
Write-Host "WiFi credentials updated in sdkconfig.defaults" -ForegroundColor Green
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Run: .\build_and_flash.ps1" -ForegroundColor Cyan
Write-Host "2. Open serial monitor to get IP address" -ForegroundColor Cyan
Write-Host "3. Open browser to http://<IP_ADDRESS>/" -ForegroundColor Cyan
Write-Host ""
