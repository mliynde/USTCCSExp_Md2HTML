#include "../include/convert.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "m2h_utils.c"
#include "gettime.c"

void repeat_text(int cnt, char *text, FILE *outfile){
	for(int i=1; i<=cnt; ++i){
		fprintf(outfile, "%s", text);
	}
}

int max_num(int a, int b){
	return a>b?a:b;
}

int get_list_level(int type, int pos, int a[]){
	for(int i=pos; i>0; --i){
		if (a[i] == type){
			return i;
		}
	}
	return 0;
}

void convert_markdown_to_html(const char *input_file) {
	clock_t start_time = clock();

	char output_file[MAX_LINE_LENGTH];
	char datetime[MAX_TIME_LENGTH];
	get_current_datetime(datetime, sizeof(datetime));

	const char *base_name = strrchr(input_file, '/');
	if (!base_name) {
		base_name = strrchr(input_file, '\\');
	}
	if (!base_name) {
		base_name = input_file;
	} else {
		base_name++;
	}

	char file_name[MAX_LINE_LENGTH];
	strncpy(file_name, base_name, MAX_LINE_LENGTH);

	char * pos_dot = file_name + strlen(file_name) - 1;
	char * file_ext = NULL;
	while(pos_dot >= file_name){
		if (*pos_dot == '.'){
			*pos_dot = '\0';
			file_ext = pos_dot + 1;
			break;
		}
		--pos_dot;
	}

	if (file_ext == NULL || strlen(file_ext) == 0 || (strcmp(file_ext, "md") != 0 && strcmp(file_ext, "markdown") != 0 && strcmp(file_ext, "MD") != 0)) {
		fprintf(stderr, "Error: No file extension like \".md\" found in %s\n", input_file);
		clock_t end_time = clock();
		double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
		fprintf(stdout, "Conversion exit in %.3f seconds.\n\n", elapsed_time);
		return;
	}

	snprintf(output_file, sizeof(output_file), "Output_%s_%s.html", file_name, datetime);

	FILE *infile = fopen(input_file, "r");
	if (infile == NULL) {
		fprintf(stderr, "Error opening input file %s\nPress any key to exit...", input_file);
		getchar();
		exit(EXIT_FAILURE);
	}
	FILE *outfile = fopen(output_file, "w");
	if (outfile == NULL) {
		fprintf(stderr, "Error opening output file %s\nPress any key to exit...", output_file);
		fclose(infile);
		getchar();
		exit(EXIT_FAILURE);
	}

	char line[MAX_LINE_LENGTH];

	preprocess_html(outfile, file_name);

	int converting_table = 0;
	int converting_codespan = 0;
	int quote_level = 0; // 标记引用状态并记录引用级别
	
	int converting_list = 0;
	int ul_level = 0, ol_level = 0;
	int list_level = 0;

	int list_level_of_order[MAX_LIST_LEVEL] = {0};

	int md_line_cnt = 0;

	while (fgets(line, sizeof(line), infile)) {
		md_line_cnt++;
		if (md_line_cnt % 100 == 0){
			fprintf(stdout, "line %d read\n", md_line_cnt);
		}

		line[strcspn(line, "\n")] = '\0';

		if (converting_table) {
			if (!is_table_row(line)) {
				fprintf(outfile, "</table>\n");
				converting_table = 0;
			}
		}

		if (quote_level){
			int cur_quote_level = is_quote(line);
			int quote_diff = cur_quote_level - quote_level;
			if (quote_diff < 0){
				repeat_text(-quote_diff, "</blockquote>\n", outfile);
				quote_level = cur_quote_level;
			}
		}

		if (converting_codespan) {	// 所谓line54
			if (is_code_block(line)) {
				fprintf(outfile, "</code></pre>\n");
				converting_codespan = 0;
				continue;
			}
			else {
				char *buffer = (char *)malloc(MAX_LINE_LENGTH * 2);
				strcpy(buffer, line);
				if (quote_level){
					buffer = find_quote_content(buffer, quote_level);
				}

				replace_special_chars(buffer); // declared in m2h_utils.c

				fprintf(outfile, "%s\n", buffer);
				free(buffer);

				continue;
			}
		}

		if (converting_list){
			int cur_ol_level = is_ordered_list(line);
			int cur_ul_level = is_unordered_list(line);
			if (cur_ol_level && cur_ol_level < list_level){
				for(int i=list_level; i>cur_ol_level; --i){
					if (list_level_of_order[i] == ORDERED){
						fprintf(outfile, "</ol>\n");
					}
					else if (list_level_of_order[i] == UNORDERED){
						fprintf(outfile, "</ul>\n");
					}
					list_level_of_order[i] = 0;
				}
				if (list_level_of_order[cur_ol_level] == UNORDERED){
					fprintf(outfile, "</ul>\n<ol>\n");
					list_level_of_order[cur_ol_level] = ORDERED;
				}
				list_level = cur_ol_level;
				ol_level = cur_ol_level;
				ul_level = get_list_level(UNORDERED, list_level, list_level_of_order);
			}
			else if(cur_ul_level && cur_ul_level < list_level){
				for(int i=list_level; i>cur_ul_level; --i){
					if (list_level_of_order[i] == ORDERED){
						fprintf(outfile, "</ol>\n");
					}
					else if (list_level_of_order[i] == UNORDERED){
						fprintf(outfile, "</ul>\n");
					}
					list_level_of_order[i] = 0;
				}
				if (list_level_of_order[cur_ul_level] == ORDERED){
					fprintf(outfile, "</ol>\n<ul>\n");
					list_level_of_order[cur_ul_level] = UNORDERED;
				}
				list_level = cur_ul_level;
				ul_level = cur_ul_level;
				ol_level = get_list_level(ORDERED, list_level, list_level_of_order);
			}
			else if(!cur_ol_level && !cur_ul_level){
				for(int i=list_level; i>0; --i){
					if (list_level_of_order[i] == ORDERED){
						fprintf(outfile, "</ol>\n");
					}
					else if (list_level_of_order[i] == UNORDERED){
						fprintf(outfile, "</ul>\n");
					}
					list_level_of_order[i] = 0;
				}
				list_level = 0;
				ol_level = 0;
				ul_level = 0;
				converting_list = 0;
			}
		}

		if (line[0] == '\0') {
			// fprintf(outfile, "<p>&nbsp;</p>\n");
		}
		else if (is_html_comment(line)) {	// 处理html注释
			fprintf(outfile, "%s\n", line);
		}
		else if (is_horizontal_rule(line)) {
			fprintf(outfile, "<hr>\n");
		}
		else if (is_header(line)) {	// 处理标题
			convert_header_to_html(outfile, line);
		}
		else if (is_quote(line)) {	// 处理引用
			int cur_quote_level = is_quote(line);
			int quote_diff = cur_quote_level - quote_level;
			if (quote_diff > 0){
				repeat_text(quote_diff, "<blockquote>\n", outfile);
			}
			char *quote_content = find_quote_content(line, cur_quote_level);
			convert_para_to_html(outfile, quote_content);

			quote_level = cur_quote_level;
		}
		else if (is_code_block(line)) { // 开始处理代码块
			if (!converting_codespan) {
				fprintf(outfile, "<pre><code>\n"); // 尚未指定代码块语言
				converting_codespan = 1;
			}
			// else 部分见 line54
		}
		else if (is_ordered_list(line)) {
			int cur_list_level = is_ordered_list(line);
			int list_diff = cur_list_level - list_level;
			if (list_diff > 0) {
				repeat_text(list_diff, "<ol>\n", outfile);
			}

			fprintf(outfile, "<li>");
			convert_para_to_html(outfile, find_ordered_list_content(line)); // 跳过列表标记和空格
			fprintf(outfile, "</li>\n");

			converting_list = 1;
			for(int i=1; i<=list_diff; ++i){
				list_level_of_order[list_level + i] = ORDERED;
			}
			ol_level = cur_list_level;
			list_level = cur_list_level;
		}
		else if (is_unordered_list(line)) {
			int cur_list_level = is_unordered_list(line);
			int list_diff = cur_list_level - list_level;
			if (list_diff > 0) {
				repeat_text(list_diff, "<ul>\n", outfile);
			}
			fprintf(outfile, "<li>");
			convert_para_to_html(outfile, find_unordered_list_content(line)); // 跳过列表标记和空格
			fprintf(outfile, "</li>\n");

			converting_list = 1;
			for(int i=1; i<=list_diff; ++i){
				list_level_of_order[list_level + i] = UNORDERED;
			}
			ul_level = cur_list_level;
			list_level = cur_list_level;
		}
		else if (is_table_row(line)) {	// 处理表格
			if (!converting_table) {
				fprintf(outfile, "<table>\n");
				converting_table = 1;
				convert_bold_table_row_to_html(outfile, line);
			}
			else {
				if (!is_table_separator(line)) {
					convert_table_row_to_html(outfile, line);
				}
			}
		}
		else {
			converting_table = 0;
			quote_level = 0;
			convert_para_to_html(outfile, line);
		}
	}

	fprintf(outfile, "</body>\n</html>\n");

	fprintf(stdout, "Successfully generated HTML file in %s\n", output_file);

	fclose(infile);
	fclose(outfile);

	clock_t end_time = clock();
	double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
	fprintf(stdout, "Conversion completed in %.3f seconds.\n\n", elapsed_time);
}