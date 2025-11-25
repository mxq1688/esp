#include "include/api_handlers.h"
#include "include/camera_driver.h"
#include "include/ml307r_driver.h"
#include "include/image_processor.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_heap_caps.h"
#include <string.h>

static const char *TAG = "API";

#define PART_BOUNDARY "123456789000000000000987654321"

// 注册所有API处理器
esp_err_t api_handlers_register(httpd_handle_t server)
{
    if (server == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // 系统状态API
    httpd_uri_t status_uri = {
        .uri = "/api/status",
        .method = HTTP_GET,
        .handler = api_status_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &status_uri);

    // 摄像头配置API
    httpd_uri_t camera_config_uri = {
        .uri = "/api/camera/config",
        .method = HTTP_GET,
        .handler = api_camera_config_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &camera_config_uri);

    // 摄像头流API
    httpd_uri_t camera_stream_uri = {
        .uri = "/api/camera/stream",
        .method = HTTP_GET,
        .handler = api_camera_stream_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &camera_stream_uri);

    // 摄像头抓拍API
    httpd_uri_t camera_capture_uri = {
        .uri = "/api/camera/capture",
        .method = HTTP_GET,
        .handler = api_camera_capture_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &camera_capture_uri);

    // 网络信息API
    httpd_uri_t network_info_uri = {
        .uri = "/api/network/info",
        .method = HTTP_GET,
        .handler = api_network_info_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &network_info_uri);

    // 热点控制API
    httpd_uri_t hotspot_control_uri = {
        .uri = "/api/hotspot/control",
        .method = HTTP_POST,
        .handler = api_hotspot_control_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &hotspot_control_uri);

    // 摄像头质量设置API
    httpd_uri_t camera_quality_uri = {
        .uri = "/api/camera/quality",
        .method = HTTP_GET,
        .handler = api_camera_config_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &camera_quality_uri);

    // 摄像头分辨率设置API
    httpd_uri_t camera_resolution_uri = {
        .uri = "/api/camera/resolution",
        .method = HTTP_GET,
        .handler = api_camera_config_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &camera_resolution_uri);

    ESP_LOGI(TAG, "✅ All API handlers registered");
    return ESP_OK;
}

// 系统状态API处理器
esp_err_t api_status_handler(httpd_req_t *req)
{
    char response[512];
    
    snprintf(response, sizeof(response),
        "{"
        "\"camera_ready\":%s,"
        "\"network_connected\":%s,"
        "\"signal_strength\":%d,"
        "\"free_heap\":%lu,"
        "\"ml307r_state\":%d"
        "}",
        camera_driver_is_ready() ? "true" : "false",
        ml307r_is_ready() ? "true" : "false",
        ml307r_get_signal_strength(),
        esp_get_free_heap_size(),
        ml307r_get_state()
    );

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
}

// 摄像头配置API处理器
esp_err_t api_camera_config_handler(httpd_req_t *req)
{
    // 解析查询参数
    char buf[128];
    size_t buf_len = httpd_req_get_url_query_len(req) + 1;
    
    if (buf_len > 1 && buf_len < sizeof(buf)) {
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            char param[32];
            
            // 检查是否是质量设置
            if (httpd_query_key_value(buf, "value", param, sizeof(param)) == ESP_OK) {
                if (strstr(req->uri, "quality") != NULL) {
                    int quality = atoi(param);
                    camera_driver_set_quality(quality);
                    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
                    return ESP_OK;
                } else if (strstr(req->uri, "resolution") != NULL) {
                    framesize_t size = FRAMESIZE_SVGA;
                    if (strcmp(param, "QVGA") == 0) size = FRAMESIZE_QVGA;
                    else if (strcmp(param, "VGA") == 0) size = FRAMESIZE_VGA;
                    else if (strcmp(param, "SVGA") == 0) size = FRAMESIZE_SVGA;
                    
                    camera_driver_set_framesize(size);
                    httpd_resp_send(req, "OK", HTTPD_RESP_USE_STRLEN);
                    return ESP_OK;
                }
            }
        }
    }

    // 返回当前配置
    camera_config_ex_t config;
    camera_driver_get_config(&config);
    
    char response[256];
    snprintf(response, sizeof(response),
        "{"
        "\"frame_size\":%d,"
        "\"pixel_format\":%d,"
        "\"jpeg_quality\":%d,"
        "\"fb_count\":%d"
        "}",
        config.frame_size,
        config.pixel_format,
        config.jpeg_quality,
        config.fb_count
    );

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
}

// 摄像头流API处理器 (MJPEG流)
esp_err_t api_camera_stream_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "Starting camera stream...");

    if (!camera_driver_is_ready()) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Camera not ready");
        return ESP_FAIL;
    }

    // 设置响应头
    httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=" PART_BOUNDARY);
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache, no-store, must-revalidate");

    // 流式传输
    esp_err_t ret = ESP_OK;
    while (true) {
        camera_fb_t *fb = camera_driver_capture();
        if (fb == NULL) {
            ESP_LOGE(TAG, "Camera capture failed");
            break;
        }

        // 发送multipart头
        char part_buf[128];
        snprintf(part_buf, sizeof(part_buf),
                 "\r\n--" PART_BOUNDARY "\r\n"
                 "Content-Type: image/jpeg\r\n"
                 "Content-Length: %zu\r\n\r\n",
                 fb->len);

        ret = httpd_resp_send_chunk(req, part_buf, strlen(part_buf));
        if (ret != ESP_OK) {
            camera_driver_release_frame(fb);
            break;
        }

        // 发送图像数据
        ret = httpd_resp_send_chunk(req, (const char *)fb->buf, fb->len);
        camera_driver_release_frame(fb);
        
        if (ret != ESP_OK) {
            break;
        }

        // 控制帧率
        vTaskDelay(pdMS_TO_TICKS(100)); // 约10fps
    }

    // 发送结束标记
    httpd_resp_send_chunk(req, "\r\n--" PART_BOUNDARY "--\r\n", strlen("\r\n--" PART_BOUNDARY "--\r\n"));

    ESP_LOGI(TAG, "Camera stream ended");
    return ret;
}

// 摄像头抓拍API处理器
esp_err_t api_camera_capture_handler(httpd_req_t *req)
{
    if (!camera_driver_is_ready()) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Camera not ready");
        return ESP_FAIL;
    }

    camera_fb_t *fb = camera_driver_capture();
    if (fb == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to capture image");
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    
    esp_err_t ret = httpd_resp_send(req, (const char *)fb->buf, fb->len);
    camera_driver_release_frame(fb);

    return ret;
}

// 网络信息API处理器
esp_err_t api_network_info_handler(httpd_req_t *req)
{
    ml307r_network_info_t info;
    char response[512];

    if (ml307r_is_ready() && ml307r_get_network_info(&info) == ESP_OK) {
        snprintf(response, sizeof(response),
            "{"
            "\"operator\":\"%s\","
            "\"network_type\":\"%s\","
            "\"signal_strength\":%d,"
            "\"is_connected\":%s,"
            "\"ip_address\":\"%s\""
            "}",
            info.operator_name,
            info.network_type,
            info.signal_strength,
            info.is_connected ? "true" : "false",
            info.ip_address
        );
    } else {
        snprintf(response, sizeof(response),
            "{"
            "\"error\":\"Network info not available\","
            "\"ml307r_ready\":%s"
            "}",
            ml307r_is_ready() ? "true" : "false"
        );
    }

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
}

// 热点控制API处理器
esp_err_t api_hotspot_control_handler(httpd_req_t *req)
{
    // 读取POST数据
    char buf[256];
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid request");
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    // 简单解析JSON（实际项目应使用cJSON）
    ESP_LOGI(TAG, "Hotspot control request: %s", buf);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

