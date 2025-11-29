#ifndef WEB_FILES_H
#define WEB_FILES_H

#include <stddef.h>

// 嵌入式HTML内容声明
extern const char index_html_content[];

// 函数声明
size_t get_index_html_size(void);
const char* get_index_html_content(void);

#endif // WEB_FILES_H
