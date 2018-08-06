/**
* Know Your Tools
* CS 241 - Fall 2017
*/
#pragma once

#include "read_wrap.h"
#include "shred.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*
* Prints how to use this utility
*/
void print_usage(char *filename);

/*
* Writes the first five lines and a newline
* to the file descriptor 'fd'
*/
void print_head(file *input, int fd);

/*
* Writes the last five lines and a newline
* to the file descriptor 'fd'
*/
void print_tail(file *input, int fd);

/*
* Writes the entire file to 'fd'
*/
void print_entire_file(file *input, int fd);
