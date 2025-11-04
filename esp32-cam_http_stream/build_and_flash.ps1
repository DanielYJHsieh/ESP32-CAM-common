# ESP32-CAM HTTP Stream - Build and Flash Script
# Usage: .\build_and_flash.ps1 [build|flash|monitor|all]

param(
    [string]$Action = "all",
    [string]$Port = "COM3"
)

# Set error handling
$ErrorActionPreference = "Stop"

Write-Host "==================================" -ForegroundColor Cyan
Write-Host "ESP32-CAM HTTP Stream Builder" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
Write-Host ""

# Check if ESP-IDF environment is loaded
if (-not $env:IDF_PATH) {
    Write-Host "Loading ESP-IDF environment..." -ForegroundColor Yellow
    $IDF_PATH = "C:\Users\danielyjhsieh\esp\v5.5.1\esp-idf"
    if (Test-Path "$IDF_PATH\export.ps1") {
        . "$IDF_PATH\export.ps1"
    } else {
        Write-Host "Error: ESP-IDF not found at $IDF_PATH" -ForegroundColor Red
        exit 1
    }
}

# Change to project directory
$ProjectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ProjectDir

Write-Host "Project directory: $ProjectDir" -ForegroundColor Green
Write-Host ""

function Build-Project {
    Write-Host "Building project..." -ForegroundColor Yellow
    idf.py build
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Build failed!" -ForegroundColor Red
        exit 1
    }
    Write-Host "Build successful!" -ForegroundColor Green
    Write-Host ""
}

function Flash-Project {
    Write-Host "Flashing to $Port..." -ForegroundColor Yellow
    Write-Host "Make sure GPIO 0 is connected to GND!" -ForegroundColor Red
    Start-Sleep -Seconds 2
    idf.py -p $Port flash
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Flash failed!" -ForegroundColor Red
        exit 1
    }
    Write-Host "Flash successful!" -ForegroundColor Green
    Write-Host ""
}

function Monitor-Project {
    Write-Host "Starting monitor on $Port..." -ForegroundColor Yellow
    Write-Host "Press Ctrl+] to exit monitor" -ForegroundColor Cyan
    Write-Host ""
    idf.py -p $Port monitor
}

# Execute requested action
switch ($Action.ToLower()) {
    "build" {
        Build-Project
    }
    "flash" {
        Flash-Project
    }
    "monitor" {
        Monitor-Project
    }
    "all" {
        Build-Project
        Flash-Project
        Monitor-Project
    }
    default {
        Write-Host "Unknown action: $Action" -ForegroundColor Red
        Write-Host "Usage: .\build_and_flash.ps1 [build|flash|monitor|all]" -ForegroundColor Yellow
        exit 1
    }
}

Write-Host ""
Write-Host "==================================" -ForegroundColor Cyan
Write-Host "Done!" -ForegroundColor Cyan
Write-Host "==================================" -ForegroundColor Cyan
