/*
 * Web Files - 从外部文件读取HTML, CSS, 和 JavaScript
 * 适配PC和H5设备的响应式设计
 */

#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "web_files.h"

// 从文件读取内容的函数
char* read_file_content(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        return NULL;
    }
    
    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // 分配内存
    char* content = (char*)malloc(file_size + 1);
    if (!content) {
        fclose(file);
        return NULL;
    }
    
    // 读取文件内容
    size_t bytes_read = fread(content, 1, file_size, file);
    content[bytes_read] = '\0';
    
    fclose(file);
    return content;
}

// 获取HTML内容
const char* get_index_html(void) {
    static char* html_content = NULL;
    
    if (!html_content) {
        html_content = read_file_content("/spiffs/index.html");
        if (!html_content) {
            // 如果无法读取文件，返回默认内容
            return get_default_html();
        }
    }
    
    return html_content;
}

// 获取CSS内容
const char* get_style_css(void) {
    static char* css_content = NULL;
    
    if (!css_content) {
        css_content = read_file_content("/spiffs/style.css");
        if (!css_content) {
            // 如果无法读取文件，返回默认内容
            return get_default_css();
        }
    }
    
    return css_content;
}

// 获取JavaScript内容
const char* get_script_js(void) {
    static char* js_content = NULL;
    
    if (!js_content) {
        js_content = read_file_content("/spiffs/script.js");
        if (!js_content) {
            // 如果无法读取文件，返回默认内容
            return get_default_js();
        }
    }
    
    return js_content;
}

// 获取PWA Manifest内容
const char* get_manifest_json(void) {
    static char* manifest_content = NULL;
    
    if (!manifest_content) {
        manifest_content = read_file_content("/spiffs/manifest.json");
        if (!manifest_content) {
            // 如果无法读取文件，返回默认内容
            return get_default_manifest();
        }
    }
    
    return manifest_content;
}

// 获取Service Worker内容
const char* get_sw_js(void) {
    static char* sw_content = NULL;
    
    if (!sw_content) {
        sw_content = read_file_content("/spiffs/sw.js");
        if (!sw_content) {
            // 如果无法读取文件，返回默认内容
            return get_default_sw();
        }
    }
    
    return sw_content;
}

// 获取文件大小的函数
size_t get_index_html_size(void) {
    const char* content = get_index_html();
    return content ? strlen(content) : 0;
}

size_t get_style_css_size(void) {
    const char* content = get_style_css();
    return content ? strlen(content) : 0;
}

size_t get_script_js_size(void) {
    const char* content = get_script_js();
    return content ? strlen(content) : 0;
}

size_t get_manifest_json_size(void) {
    const char* content = get_manifest_json();
    return content ? strlen(content) : 0;
}

size_t get_sw_js_size(void) {
    const char* content = get_sw_js();
    return content ? strlen(content) : 0;
}

// 默认HTML内容（作为备用）
const char* get_default_html(void) {
    return "<!DOCTYPE html><html><head><title>ESP32-C3 LED控制器</title></head><body><h1>LED控制器</h1><p>请确保web文件已正确上传到SPIFFS</p></body></html>";
}

// 默认CSS内容（作为备用）
const char* get_default_css(void) {
    return "body { font-family: Arial, sans-serif; text-align: center; padding: 50px; }";
}

// 默认JavaScript内容（作为备用）
const char* get_default_js(void) {
    return "console.log('LED控制器已加载');";
}

// 默认Manifest内容（作为备用）
const char* get_default_manifest(void) {
    return "{\"name\":\"LED控制器\",\"short_name\":\"LED控制器\"}";
}

// 默认Service Worker内容（作为备用）
const char* get_default_sw(void) {
    return "// Default Service Worker";
}

// 清理函数
void cleanup_web_files(void) {
    // 这里可以添加清理逻辑
}
