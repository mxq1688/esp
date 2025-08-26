#ifndef WEB_FILES_H
#define WEB_FILES_H

#include <stddef.h>

// Web文件声明
extern const char index_html_start[] asm("_binary_index_html_start");
extern const char index_html_end[] asm("_binary_index_html_end");
extern const char manifest_json_start[] asm("_binary_manifest_json_start");
extern const char manifest_json_end[] asm("_binary_manifest_json_end");
extern const char script_js_start[] asm("_binary_script_js_start");
extern const char script_js_end[] asm("_binary_script_js_end");
extern const char style_css_start[] asm("_binary_style_css_start");
extern const char style_css_end[] asm("_binary_style_css_end");
extern const char sw_js_start[] asm("_binary_sw_js_start");
extern const char sw_js_end[] asm("_binary_sw_js_end");

// 获取文件大小的宏
#define GET_FILE_SIZE(start, end) ((size_t)(end - start))

#endif // WEB_FILES_H
