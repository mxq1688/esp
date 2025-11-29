#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// 嵌入的Web文件声明
extern const char web_index_html_start[] asm("_binary_index_html_start");
extern const char web_index_html_end[] asm("_binary_index_html_end");

extern const char web_style_css_start[] asm("_binary_style_css_start");
extern const char web_style_css_end[] asm("_binary_style_css_end");

extern const char web_script_js_start[] asm("_binary_script_js_start");
extern const char web_script_js_end[] asm("_binary_script_js_end");

// 获取文件大小的宏
#define WEB_INDEX_HTML_SIZE (web_index_html_end - web_index_html_start)
#define WEB_STYLE_CSS_SIZE (web_style_css_end - web_style_css_start)
#define WEB_SCRIPT_JS_SIZE (web_script_js_end - web_script_js_start)

#ifdef __cplusplus
}
#endif
