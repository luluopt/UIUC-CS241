/**
* Know Your Tools
* CS 241 - Fall 2017
*/
#pragma once

/*
* You don't need to worry about functions in this file
* You're boss already tested them
*/

#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

/*
* Returns the size of the file in bytes
* -1 if an error occurred, errno is set
*/
ssize_t file_size(int fd);

/*
* Moves the file descriptor back to the
* beginning of the file
*/
void reset_file(int fd);

/*
* Returns the minimum of the two values;
*
*/
ssize_t min(ssize_t a, ssize_t b);