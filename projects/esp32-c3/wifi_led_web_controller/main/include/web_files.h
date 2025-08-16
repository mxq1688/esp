#ifndef WEB_FILES_H
#define WEB_FILES_H

#include <stddef.h>
#include <string.h>

// 嵌入的HTML文件内容（包含CSS和JavaScript）
extern const char index_html[];

// 获取文件大小的函数
size_t get_index_html_size(void);

#endif // WEB_FILES_H
