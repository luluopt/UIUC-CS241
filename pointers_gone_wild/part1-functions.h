/**
 * Lab: Pointers Gone Wild
 * CS 241 - Fall 2017
 */

#pragma once

void one(const char *grade);
void two();
void three(const int *x, const int *y);
float *four(const int *x);
void five(const char *a);
void six(const char *s);
void seven();
void eight(int a);
void nine(const char *s);
void ten(const int d);
long int clear_bits(long int value, long int flag);
int little_automaton(int (*transition_function)(int, char), const char *input);
