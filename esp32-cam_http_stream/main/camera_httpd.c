/*
 * ESP32-CAM HTTP MJPEG Stream Server
 * 
 * 功能:
 * - HTTP MJPEG 即時串流
 * - Web 介面查看串流
 * - 支援多客戶端連接
 * - 可調整解析度和品質
 * 
 * 硬體: ESP32-CAM (AI-Thinker)
 * 框架: ESP-IDF
 */

#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include <lwip/sockets.h>
#include <lwip/netdb.h>

static const char *TAG = "camera_httpd";

// ESP32-CAM (AI-Thinker) Pin Definition
#define CAM_PIN_PWDN    32
#define CAM_PIN_RESET   -1
#define CAM_PIN_XCLK    0
#define CAM_PIN_SIOD    26
#define CAM_PIN_SIOC    27

#define CAM_PIN_D7      35
#define CAM_PIN_D6      34
#define CAM_PIN_D5      39
#define CAM_PIN_D4      36
#define CAM_PIN_D3      21
#define CAM_PIN_D2      19
#define CAM_PIN_D1      18
#define CAM_PIN_D0      5
#define CAM_PIN_VSYNC   25
#define CAM_PIN_HREF    23
#define CAM_PIN_PCLK    22

// MJPEG Stream Settings
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

// Camera configuration
static camera_config_t camera_config = {
    .pin_pwdn  = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    .xclk_freq_hz = 20000000,        // 20MHz XCLK (maximum stable frequency)
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_UXGA,    // 1600x1200 - UXGA (Maximum for OV2640)
    .jpeg_quality = 12,              // 0-63 lower=higher quality, 12=balanced for speed
    .fb_count = 3,                   // Triple buffer for maximum FPS with PSRAM
    .fb_location = CAMERA_FB_IN_PSRAM, // Use PSRAM for frame buffers
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY
};

typedef struct {
    httpd_req_t *req;
    size_t len;
} jpg_chunking_t;

// Check if client is from local network
static bool is_local_client(httpd_req_t *req)
{
    int sockfd = httpd_req_to_sockfd(req);
    struct sockaddr_in6 addr;
    socklen_t addr_size = sizeof(addr);
    
    if (getpeername(sockfd, (struct sockaddr *)&addr, &addr_size) != 0) {
        ESP_LOGW(TAG, "Failed to get peer address");
        return false;
    }
    
    // Get ESP32's IP address
    esp_netif_ip_info_t ip_info;
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif == NULL || esp_netif_get_ip_info(netif, &ip_info) != ESP_OK) {
        ESP_LOGW(TAG, "Failed to get local IP info");
        return false;
    }
    
    // Handle IPv4-mapped IPv6 addresses
    uint32_t client_ip;
    if (addr.sin6_family == AF_INET6) {
        // Check if it's IPv4-mapped IPv6 (::ffff:x.x.x.x)
        uint8_t *addr_bytes = (uint8_t *)&addr.sin6_addr;
        if (addr_bytes[10] == 0xff && addr_bytes[11] == 0xff) {
            // Extract IPv4 address from IPv4-mapped IPv6
            client_ip = *((uint32_t *)(addr_bytes + 12));
        } else {
            ESP_LOGW(TAG, "Pure IPv6 not supported for local check");
            return false;
        }
    } else if (addr.sin6_family == AF_INET) {
        struct sockaddr_in *addr_in = (struct sockaddr_in *)&addr;
        client_ip = addr_in->sin_addr.s_addr;
    } else {
        ESP_LOGW(TAG, "Unknown address family");
        return false;
    }
    
    // Compare network portion (assuming /24 subnet)
    uint32_t local_ip = ip_info.ip.addr;
    uint32_t netmask = ip_info.netmask.addr;
    
    bool is_local = (client_ip & netmask) == (local_ip & netmask);
    
    if (!is_local) {
        ESP_LOGW(TAG, "Access denied: Client IP 0x%08lx not in local network (0x%08lx/0x%08lx)",
                 (unsigned long)client_ip, (unsigned long)local_ip, (unsigned long)netmask);
    }
    
    return is_local;
}

// Check PSRAM availability
static void check_psram()
{
    ESP_LOGI(TAG, "=== PSRAM Diagnostic ===");
    
    // Check if PSRAM is available using heap caps
    size_t total_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t largest_free = heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM);
    
    ESP_LOGI(TAG, "PSRAM Total: %d bytes (%.2f MB)", total_psram, total_psram / (1024.0 * 1024.0));
    ESP_LOGI(TAG, "PSRAM Free: %d bytes (%.2f MB)", free_psram, free_psram / (1024.0 * 1024.0));
    ESP_LOGI(TAG, "PSRAM Used: %d bytes (%.2f MB)", total_psram - free_psram, (total_psram - free_psram) / (1024.0 * 1024.0));
    ESP_LOGI(TAG, "PSRAM Largest Free Block: %d bytes (%.2f MB)", largest_free, largest_free / (1024.0 * 1024.0));
    
    if (total_psram == 0) {
        ESP_LOGE(TAG, "PSRAM not detected or not enabled!");
        return;
    }
    
    // Try to allocate a small test buffer
    ESP_LOGI(TAG, "Testing PSRAM allocation...");
    void *test_buf = heap_caps_malloc(1024, MALLOC_CAP_SPIRAM);
    if (test_buf) {
        ESP_LOGI(TAG, "✓ PSRAM test allocation successful (1KB)");
        heap_caps_free(test_buf);
    } else {
        ESP_LOGE(TAG, "✗ PSRAM test allocation failed!");
    }
    
    // Try larger allocation (16KB)
    test_buf = heap_caps_malloc(16384, MALLOC_CAP_SPIRAM);
    if (test_buf) {
        ESP_LOGI(TAG, "✓ PSRAM allocation successful (16KB)");
        heap_caps_free(test_buf);
    } else {
        ESP_LOGE(TAG, "✗ PSRAM allocation failed (16KB)!");
    }
    
    // Try 64KB allocation
    test_buf = heap_caps_malloc(65536, MALLOC_CAP_SPIRAM);
    if (test_buf) {
        ESP_LOGI(TAG, "✓ PSRAM large allocation successful (64KB)");
        heap_caps_free(test_buf);
    } else {
        ESP_LOGE(TAG, "✗ PSRAM large allocation failed (64KB)!");
        ESP_LOGE(TAG, "Largest free block is only: %d bytes", largest_free);
    }
    
    ESP_LOGI(TAG, "========================");
}

// Initialize camera
static esp_err_t init_camera()
{
    ESP_LOGI(TAG, "Checking PSRAM before camera initialization...");
    check_psram();
    
    ESP_LOGI(TAG, "Initializing camera with PSRAM...");
    
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed with error 0x%x", err);
        return err;
    }
    
    // Get camera sensor
    sensor_t * s = esp_camera_sensor_get();
    if (s != NULL) {
        // Initial adjustments
        s->set_brightness(s, 0);     // -2 to 2
        s->set_contrast(s, 0);       // -2 to 2
        s->set_saturation(s, 0);     // -2 to 2
        s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect)
        s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
        s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
        s->set_wb_mode(s, 0);        // 0 to 4
        s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
        s->set_aec2(s, 0);           // 0 = disable , 1 = enable
        s->set_ae_level(s, 0);       // -2 to 2
        s->set_aec_value(s, 300);    // 0 to 1200
        s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
        s->set_agc_gain(s, 0);       // 0 to 30
        s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
        s->set_bpc(s, 0);            // 0 = disable , 1 = enable
        s->set_wpc(s, 1);            // 0 = disable , 1 = enable
        s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
        s->set_lenc(s, 1);           // 0 = disable , 1 = enable
        s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
        s->set_vflip(s, 0);          // 0 = disable , 1 = enable
        s->set_dcw(s, 1);            // 0 = disable , 1 = enable
        s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
    }
    
    ESP_LOGI(TAG, "Camera initialized successfully");
    return ESP_OK;
}

// MJPEG Stream Handler
static esp_err_t stream_handler(httpd_req_t *req)
{
    // Check if client is from local network
    if (!is_local_client(req)) {
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "Access denied: Only local network access allowed");
        return ESP_FAIL;
    }
    
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t * _jpg_buf = NULL;
    char part_buf[64];
    
    ESP_LOGI(TAG, "Stream session started");
    
    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK){
        ESP_LOGE(TAG, "Failed to set response type");
        return res;
    }
    
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "X-Framerate", "10");
    
    while(true){
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
            break;
        }
        
        if(fb->format != PIXFORMAT_JPEG){
            bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
            esp_camera_fb_return(fb);
            fb = NULL;
            if(!jpeg_converted){
                ESP_LOGE(TAG, "JPEG compression failed");
                res = ESP_FAIL;
                break;
            }
        } else {
            _jpg_buf_len = fb->len;
            _jpg_buf = fb->buf;
        }
        
        if(res == ESP_OK){
            size_t hlen = snprintf(part_buf, 64, _STREAM_PART, _jpg_buf_len);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        
        if(fb){
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        } else if(_jpg_buf){
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        
        if(res != ESP_OK){
            ESP_LOGI(TAG, "Client disconnected");
            break;
        }
        
        // Small delay to prevent CPU overload
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    ESP_LOGI(TAG, "Stream session ended");
    return res;
}

// Capture single image handler
static esp_err_t capture_handler(httpd_req_t *req)
{
    // Check if client is from local network
    if (!is_local_client(req)) {
        httpd_resp_send_err(req, HTTPD_403_FORBIDDEN, "Access denied: Only local network access allowed");
        return ESP_FAIL;
    }
    
    camera_fb_t * fb = NULL;
    esp_err_t res = ESP_OK;
    
    // Flush old frames from buffer (fb_count = 3, so flush 3 times)
    for (int i = 0; i < 3; i++) {
        fb = esp_camera_fb_get();
        if (fb) {
            esp_camera_fb_return(fb);
        }
    }
    
    // Now get the fresh frame
    fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    
    res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
    esp_camera_fb_return(fb);
    return res;
}

// Status handler
static esp_err_t status_handler(httpd_req_t *req)
{
    static char json_response[1024];
    
    sensor_t * s = esp_camera_sensor_get();
    char * p = json_response;
    *p++ = '{';
    
    p+=sprintf(p, "\"framesize\":%u,", s->status.framesize);
    p+=sprintf(p, "\"quality\":%u,", s->status.quality);
    p+=sprintf(p, "\"brightness\":%d,", s->status.brightness);
    p+=sprintf(p, "\"contrast\":%d,", s->status.contrast);
    p+=sprintf(p, "\"saturation\":%d,", s->status.saturation);
    p+=sprintf(p, "\"hmirror\":%u,", s->status.hmirror);
    p+=sprintf(p, "\"vflip\":%u", s->status.vflip);
    *p++ = '}';
    *p++ = 0;
    
    httpd_resp_set_type(req, "application/json");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    return httpd_resp_send(req, json_response, strlen(json_response));
}

// Index page HTML
static const char INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>ESP32-CAM Control</title>
    <style>
        body {
            font-family: Arial, Helvetica, sans-serif;
            background: #181818;
            color: #EFEFEF;
            margin: 0;
            padding: 0;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }
        h1 {
            text-align: center;
            color: #4CAF50;
        }
        .controls {
            text-align: center;
            margin: 30px 0;
        }
        button {
            background-color: #4CAF50;
            border: none;
            color: white;
            padding: 20px 40px;
            text-align: center;
            text-decoration: none;
            display: inline-block;
            font-size: 18px;
            margin: 10px;
            cursor: pointer;
            border-radius: 8px;
            transition: all 0.3s;
        }
        button:hover {
            background-color: #45a049;
            transform: scale(1.05);
        }
        button:active {
            transform: scale(0.95);
        }
        button.stop {
            background-color: #f44336;
        }
        button.stop:hover {
            background-color: #da190b;
        }
        .stream-container {
            text-align: center;
            margin: 20px 0;
            min-height: 400px;
        }
        #display {
            max-width: 100%;
            height: auto;
            border: 2px solid #4CAF50;
            border-radius: 8px;
            display: none;
        }
        .info {
            background: #282828;
            padding: 15px;
            border-radius: 8px;
            margin: 20px 0;
            text-align: center;
        }
        .info p {
            margin: 5px 0;
        }
        #status {
            font-weight: bold;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎥 ESP32-CAM Control Panel</h1>
        
        <div class="controls">
            <button onclick="startStream();">📹 Stream</button>
            <button onclick="stopStream();" class="stop">⏹ Stop</button>
            <button onclick="captureImage();">📷 Capture</button>
        </div>
        
        <div class="stream-container">
            <img id="display">
        </div>
        
        <div class="info">
            <p><strong>Status:</strong> <span id="status" style="color: #4CAF50;">Ready</span></p>
            <p><strong>IP:</strong> <span id="ip"></span></p>
        </div>
    </div>
    
    <script>
        const display = document.getElementById('display');
        const statusText = document.getElementById('status');
        let isStreaming = false;
        let streamUrl = '';
        
        function startStream() {
            if (!isStreaming) {
                streamUrl = '/stream?t=' + new Date().getTime();
                display.src = streamUrl;
                display.style.display = 'block';
                isStreaming = true;
                display.onload = function() {
                    statusText.textContent = 'Streaming';
                    statusText.style.color = '#4CAF50';
                };
                display.onerror = function() {
                    statusText.textContent = 'Stream Error';
                    statusText.style.color = '#f44336';
                    isStreaming = false;
                };
            }
        }
        
        function stopStream() {
            if (isStreaming) {
                // Stop streaming by clearing the source
                isStreaming = false;
                display.src = '';
                // Capture current frame before stopping
                fetch('/capture')
                    .then(response => response.blob())
                    .then(blob => {
                        display.src = URL.createObjectURL(blob);
                        display.style.display = 'block';
                        statusText.textContent = 'Stopped (Last Frame)';
                        statusText.style.color = '#FF9800';
                    })
                    .catch(error => {
                        statusText.textContent = 'Stop Error';
                        statusText.style.color = '#f44336';
                    });
            }
        }
        
        function captureImage() {
            // Stop streaming if active
            if (isStreaming) {
                isStreaming = false;
                display.src = '';
            }
            
            // Fetch and display capture
            fetch('/capture?t=' + new Date().getTime())
                .then(response => response.blob())
                .then(blob => {
                    display.src = URL.createObjectURL(blob);
                    display.style.display = 'block';
                    statusText.textContent = 'Image Captured';
                    statusText.style.color = '#2196F3';
                })
                .catch(error => {
                    statusText.textContent = 'Capture Error';
                    statusText.style.color = '#f44336';
                });
        }
        
        document.getElementById('ip').textContent = window.location.hostname;
    </script>
</body>
</html>
)rawliteral";

// Index page handler
static esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html");
    httpd_resp_set_hdr(req, "Content-Encoding", "identity");
    return httpd_resp_send(req, INDEX_HTML, strlen(INDEX_HTML));
}

// Start HTTP server
static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;
    config.ctrl_port = 32768;
    config.max_uri_handlers = 8;
    config.max_resp_headers = 8;
    config.stack_size = 8192;
    
    ESP_LOGI(TAG, "Starting web server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t index_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = index_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &index_uri);
        
        httpd_uri_t stream_uri = {
            .uri       = "/stream",
            .method    = HTTP_GET,
            .handler   = stream_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &stream_uri);
        
        httpd_uri_t capture_uri = {
            .uri       = "/capture",
            .method    = HTTP_GET,
            .handler   = capture_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &capture_uri);
        
        httpd_uri_t status_uri = {
            .uri       = "/status",
            .method    = HTTP_GET,
            .handler   = status_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &status_uri);
        
        ESP_LOGI(TAG, "Web server started successfully");
        return server;
    }
    
    ESP_LOGI(TAG, "Error starting web server!");
    return NULL;
}

// WiFi event handler
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Disconnected from WiFi, retrying...");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
    }
}

// Initialize WiFi
static void wifi_init_sta(void)
{
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi initialized. Connecting to SSID:%s", CONFIG_ESP_WIFI_SSID);
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-CAM HTTP Stream Server Starting...");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    // Initialize WiFi
    wifi_init_sta();
    
    // Wait for WiFi connection (simple delay)
    ESP_LOGI(TAG, "Waiting for WiFi connection...");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    
    // Initialize camera
    if(init_camera() != ESP_OK) {
        ESP_LOGE(TAG, "Camera initialization failed!");
        return;
    }
    
    // Start web server
    start_webserver();
    
    ESP_LOGI(TAG, "Camera stream server ready!");
    ESP_LOGI(TAG, "Access the camera at http://<IP_ADDRESS>/");
}
