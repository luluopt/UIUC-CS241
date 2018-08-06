/**
 * Machine Problem: Text Editor
 * CS 241 - Fall 2017
 */
#include "document.h"
#include "editor.h"
#include "format.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_filename(int argc, char *argv[]) {
    // TODO implement get_filename
    // take a look at editor_main.c to see what this is used for
    return argv[argc - 1];
}

document *handle_create_document(const char *path_to_file) {
    // TODO create the document
   return  document_create_from_file(path_to_file);
}


void handle_cleanup(document *document) {
    // TODO destroy the document
	document_destroy(document);
}


void handle_display_command(document *document, size_t start_line,
                            ssize_t max_lines, size_t start_col_index,
                            ssize_t max_cols) {
    // TODO implement handle_display_command
	size_t size = document_size(document);
	ssize_t max_lines_no = 0;
	if(max_lines == -1||(max_lines+(ssize_t)start_line)>(ssize_t)size){
		max_lines_no = size+1;
	}else{
		max_lines_no = (ssize_t)start_line+max_lines;
	}
	//size_t num = 1;
	
	for(ssize_t i = start_line;i<max_lines_no;i++){
		const char *current_line = document_get_line(document,i);
		ssize_t col_end;
		if(max_cols == -1 || max_cols>(ssize_t)strlen(current_line)){
			col_end = strlen(current_line);
		}else{
			col_end = start_col_index + max_cols;
		}
		//printf("%ld\t",num++);
		printf("%ld\t",i);
		for(ssize_t j = start_col_index;j<col_end;j++){
			//if(*(current_line+j)!='\n')
				printf("%c",*(current_line+j));
		}
		printf("\n");
	}
	
}

void handle_insert_command(document *document, location loc, const char *line) {
    // TODO implement handle_insert_command
	
	if(loc.line_no<=document_size(document)){
		const char *current_line = document_get_line(document, loc.line_no);
		char* temp = (char *)malloc(strlen(current_line)+strlen(line)+1);
		for(size_t i=0;i<loc.idx;i++){
			*(temp+i) = *(current_line+i);
		}
		for(size_t i=0;i<strlen(line);i++){
			*(temp+loc.idx+i) = *(line+i);
		}
		strcpy(temp+strlen(line)+loc.idx,current_line+loc.idx);
		document_set_line(document, loc.line_no, temp);
		free(temp);
	}
	else{
		document_insert_line(document, loc.line_no, line);
	}
}


void handle_append_command(document *document, size_t line_no,
                           const char *line) {
    // TODO implement handle_append_command
    if(document_size(document)==0){
    	document_insert_line(document, line_no, "");
    }
    const char *current_line = document_get_line(document, line_no);
	size_t len = strlen(line);
	size_t flag = 0;
	char *temp = (char *)malloc(len+strlen(current_line)+1);
	int num = strlen(current_line);
	size_t line_num = line_no;
	strcpy(temp, current_line);
	for(size_t i=0;i<len;i++){
		if(*(line+i)=='\\'){
			flag++;
		}
		else{
			if(flag == 1 && *(line+i)=='n'){
				//*(temp+num) = '\n';
				//num++;
				*(temp+num) = '\0';
				document_insert_line(document, line_num, temp);
				memset(temp, 0, (len+strlen(current_line))*sizeof(char));
				line_num++;
				flag = 0;
				num = 0;
			}
			else if(flag == 2){
				flag = 0;
				*(temp+num) = '\\';
				num++;
				*(temp+num) = *(line+i);
				num++;
			}
			else{
				*(temp+num) = *(line+i);
				num++;
				flag = 0;
			}
		}
	}
	if(num!=0){
		*(temp+num) = '\0';
		//*(temp+num+1) = '\0';
		document_insert_line(document, line_num, temp);
	}
	document_delete_line(document, line_num+1);
	free(temp);
}

void handle_write_command(document *document, size_t line_no,
                          const char *line) {
    // TODO implement handle_write_command
	//const char *current_line = document_get_line(document, line_no+1);
	if(document_size(document)==0){
		document_insert_line(document, line_no, "");
	}
	size_t len = strlen(line);
	size_t flag = 0; 
	char *temp = (char *)malloc(len+1);
	int num = 0;  // 
	size_t line_num = line_no;
	for(size_t i=0;i<len;i++){
		if(*(line+i)=='\\'){
			flag++;
		}
		else{
			if(flag == 1 && *(line+i)=='n'){
				//*(temp+num) = '\n';
				//num++;
				*(temp+num) = '\0';
				document_insert_line(document, line_num , temp);
				memset(temp,0,len*sizeof(char));
				line_num++;
				flag = 0;	
				num = 0;
			}
			else if(flag == 2){
				flag = 0;
				*(temp+num) = '\\';
				num++;
				*(temp+num) = *(line+i);
				num++;
			}
			else{
				*(temp+num) = *(line+i);
				num++;
				flag = 0;
			}
		}
	}
	if(num!=0){
		*(temp+num) = '\0';
		//*(temp+num+1) = '\0';
		document_insert_line(document, line_num, temp);
	}
	document_delete_line(document, line_num+1);
	free(temp);
}

void handle_delete_command(document *document, location loc, size_t num_chars) {
    // TODO implement handle_delete_command
	const char *current_line = document_get_line(document, loc.line_no);
	size_t len = strlen(current_line);
	char *temp = (char *)malloc(len);
	size_t i = 0;
	for(;i<loc.idx;i++){
		*(temp+i) = *(current_line+i);
	}
	for(size_t j = 0;j<len-num_chars-loc.idx;j++){
		*(temp+i)= *(current_line+loc.idx+num_chars+j);
		i++;
	}
	*(temp+i) = '\0';
	document_set_line(document, loc.line_no, temp);
	free(temp);
}

void handle_delete_line(document *document, size_t line_no) {
    // TODO implement handle_delete_line
    if (document_size(document)==0){
    	return ;
    }
	document_delete_line(document, line_no);	
}


location handle_search_command(document *document, location loc,
                               const char *search_str) {
    // TODO implement handle_search_command
	ssize_t col_start = loc.idx;
	size_t finish = 0;
	size_t i = loc.line_no;
	size_t size = document_size(document);
	//for(size_t i = loc.line_no; i<=document_size(document);i++){
	while(finish!=1){
		
		const char *current_line = document_get_line(document,i);
		char *p = strstr(current_line+col_start, search_str);
		col_start = 0;
		if(p!=NULL){
			return (location){i, (size_t)(p-current_line)};
		}
		else{	
			if(i == size){	
				i = 1;
			}
			else{
				i++;
				if(i == loc.line_no){	
					finish++;
				}
			}
		}
		
	}
	return (location){0,0};
}
/**
 *	合并两行命令
 *	先合并，再删除
 */
void handle_merge_line(document *document, size_t line_no) {
    // TODO implement handle_merge_line
	const char *current_line = document_get_line(document, line_no);
	const char *following_line = document_get_line(document, line_no+1);
	char *merge_line = (char *)malloc(strlen(current_line)+strlen(following_line)+1);
	strcpy(merge_line, current_line);
	strcat(merge_line, following_line);
	document_set_line(document, line_no+1 , merge_line);
	document_delete_line(document, line_no);
	free(merge_line);
}
/**
 *	将字符串分为两个子串，再插入到document中
 */
void handle_split_line(document *document, location loc) {
    // TODO implement handle_split_line
	const char *current_line = document_get_line(document, loc.line_no);
	size_t len = strlen(current_line);
	char *temp = (char *)malloc(len-loc.idx+1);
	strcpy(temp, current_line+loc.idx);
	char *temp1 = (char *)malloc(loc.idx+1);
	for(size_t i = 0;i<loc.idx;i++){
		*(temp1+i) = *(current_line+i);
	}
	//*(temp1+loc.idx) = '\n';
	*(temp1+loc.idx) = '\0';
	document_set_line(document, loc.line_no, temp1); 
	document_insert_line(document, loc.line_no+1, temp); 
	free(temp);
	free(temp1);
}

void handle_save_command(document *document, const char *filename) {
    // TODO implement handle_save_command
	document_write_to_file(document, filename);
}
