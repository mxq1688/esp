#ifndef WEB_FILES_H
#define WEB_FILES_H

#include <stddef.h>
#include <string.h>

// 嵌入的HTML文件内容
extern const char index_html[];

// 嵌入的CSS文件内容
extern const char style_css[];

// 嵌入的JavaScript文件内容
extern const char script_js[];

// PWA文件内容
extern const char manifest_json[];
extern const char sw_js[];

// 获取文件大小的函数
size_t get_index_html_size(void);
size_t get_style_css_size(void);
size_t get_script_js_size(void);
size_t get_manifest_json_size(void);
size_t get_sw_js_size(void);

#endif // WEB_FILES_H
