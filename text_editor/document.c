/**
 * Machine Problem: Text Editor
 * CS 241 - Fall 2017
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "document.h"
#include "vector.h"

struct document {
    vector *vector;
};

document *document_create() {
    document *this = (document *)malloc(sizeof(document));
    assert(this);
    this->vector = vector_create(string_copy_constructor, string_destructor,
                                 string_default_constructor);
    return this;
}

void document_write_to_file(document *this, const char *path_to_file) {
    assert(this);
    assert(path_to_file);
    // see the comment in the header file for a description of how to do this!
    // TODO: your code here!
	FILE *f;
	if((f = fopen(path_to_file,"w"))!=NULL){
		size_t size = vector_size(this->vector);
		for(size_t i=0;i<size;i++){
			fprintf(f,"%s\n",vector_get(this->vector,i));
		}
	}
	else{
		fprintf(stderr, "Failed to open this file!\n");
	}
	fclose(f);
}

document *document_create_from_file(const char *path_to_file) {
    assert(path_to_file);
    // this function will read a file which is created by document_write_to_file
    // TODO: your code here!
	document *document = document_create();
	FILE *f;
	if((f = fopen(path_to_file,"r"))!=NULL){
		char *line;
		size_t n = 0;
		while(getline(&line,&n,f)!=EOF){
			if (line[strlen(line) - 1] == '\n') {    //去掉行尾换行符
				line[strlen(line) - 1] = '\0';
			}
			vector_push_back(document->vector,line);
		}
	}
	else{
		 //fprintf(stderr,"Failed to open this file!\n");
	}
    return document;
}

void document_destroy(document *this) {
    assert(this);
    vector_destroy(this->vector);
    free(this);
}

size_t document_size(document *this) {
    assert(this);
    return vector_size(this->vector);
}

void document_set_line(document *this, size_t line_number, const char *str) {
    assert(this);
    assert(str);
    size_t index = line_number - 1;
    vector_set(this->vector, index, (void *)str);
}

const char *document_get_line(document *this, size_t line_number) {
    assert(this);
    assert(line_number > 0);
    size_t index = line_number - 1;
    return (const char *)vector_get(this->vector, index);
}

void document_insert_line(document *this, size_t line_number, const char *str) {
    assert(this);
    assert(str);
    // TODO: your code here!
    // How are you going to handle the case when the user wants to
    // insert a line past the end of the document?
	size_t lines = vector_size(this->vector);
	if(line_number<=lines){
		vector_insert(this->vector,line_number-1,(void *)str);
	}/*
	else if(line_number == lines){
		const char * current_line = document_get_line(this, line_number);
		vector_push_back(this->vector,(void *)current_line);
		document_set_line(this, line_number ,str);
	}*/
	else{
		for(size_t i=0;i<line_number - lines - 1;i++){
			vector_push_back(this->vector,"");
		}
		vector_push_back(this->vector,(void *)str);
	}
}

void document_delete_line(document *this, size_t line_number) {
    assert(this);
    assert(line_number > 0);
    size_t index = line_number - 1;
    vector_erase(this->vector, index);
}
