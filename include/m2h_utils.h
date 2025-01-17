#ifndef M2H_UTILS_H
#define M2H_UTILS_H

#ifndef MAX_LINE_LENGTH
#define MAX_LINE_LENGTH 2048
#endif // MAX_LINE_LENGTH

#ifndef REPLACEMENT_COUNT
#define REPLACEMENT_COUNT 10
#endif // REPLACEMENT_COUNT

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void preprocess_html(FILE *outfile, const char *file); // 为html文件添加头部信息

// para

int find_once(char *str, const char *tofind); // 在str中查找一次

int replace_all(char *str, const char *old, const char *newstr); // 实现全部文本替换

int replace_once(char *str, const char *old, const char *newstr); // 实现第一个目标文本替换

int insert_all(char *str, const char *old, const char *instr); // 实现全部文本插入（未修复replace_all时）

void replace_special_chars(char *str); // 在m2h时替换特殊字符，避免与html语法冲突

void convert_title_to_id(char *str); // 将标题转换为id

void convert_markdown_links(char *str); // markdown链接转换

void convert_markdown_images(char *str); // markdown图片转换

void convert_formats(char *str);

void convert_content_to_html(FILE *outfile, const char *file); // 统一内容输出

void convert_para_to_html(FILE *outfile, const char *file); // 段落输出

// end para

void convert_header_to_html(FILE *outfile, const char *line); // 将markdown标题转换为html标题

void convert_table_row_to_html(FILE *outfile, const char *line); // 将markdown表格行转换为html表格行

void convert_bold_table_row_to_html(FILE *outfile, const char *line); // 将markdown加粗行转换为html加粗行

// find_quote_content移至m2h_judge

char * find_ordered_list_content(const char *line);

char * find_unordered_list_content(const char *line);

#endif // M2H_UTILS_H