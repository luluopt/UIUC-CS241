/**
 * MP: Extreme Edge Cases
 * CS 241 - Fall 2017
 */
#include "camelCaser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Separate the input_str into sentences
char **parse_inputs(const char *input_str, int *num_sentences)
{
    *num_sentences = 0;

    size_t input_len = strlen(input_str);
    
    // can't assume the number of senstences, so unless we have a linked list
    // implementation, we have to count the number of sentences first
    // fortunately this is just the number of punctuations
    for (size_t i=0; i<input_len; i++) {
        // ispunct() return non-zero value for punctuations
        if (ispunct(input_str[i]) != 0) {
            *num_sentences += 1;
        } 
    }
   
    // Allocate structure for input sentences
    char **inputs = (char**)malloc(sizeof(char*) * (*num_sentences + 1));
    memset(inputs, 0, sizeof(char*) * (*num_sentences + 1));

    // Scan the input_str again, this time to separate the sentences
    int sentence_idx = 0;
    int start_idx    = 0;
    int len          = 0;
    for (size_t i=0; i<input_len; i++) {
        // Again find the punctuation
        if (ispunct(input_str[i]) != 0) {
            inputs[sentence_idx] = (char*)malloc(len+1); // length of the sentence + 1 char for null termination 
            memset(inputs[sentence_idx], 0, len + 1);

            memcpy(inputs[sentence_idx], input_str + start_idx, len); // copy the input over
            // printf("sentence is %s\n", inputs[sentence_idx]);

            len = 0;
            sentence_idx++;
            start_idx = i+1;  // the start idx is the next character
        } else {
            // increment the length counter if it isn't a punctuation
            len++;
        }
    }
    
    return inputs;
}

// Convert the word to camel case, write it the the output buffer provided 
void convert_word(const char *word, int len, char *output)
{
    int is_first = 1;

    for (int j=0; j<len; j++) {
        if (isalpha(word[j]) != 0) {
            // 32 is the difference between ascii values of uppercase and lowercase
            if (is_first) {
                *output = word[j] >= 'a' ? word[j] - 32 : word[j];  // assign the current output char to uppercase
            } else {
                *output = word[j] >= 'a' ? word[j] : word[j] + 32;  // assign the current output char to lowercase
            }
        } else {
            // copy over non-alpha characters directly
            *output = word[j];
        }
        if (is_first) {
            is_first = 0;
        }
        output++;  // move along the output buffer
    }
}

char *convert_sentence(const char *input_s)
{
    int parsing_space = 1;  // the idle state of the parser is a space
    int start_idx = 0;
    int len = 0;
    
    size_t sentence_len = strlen(input_s);

    // It's inefficient to determine what the output length is, but it will
    // never exceed the input length
    char  *output_s = malloc(sentence_len + 1);
    memset(output_s, 0, sentence_len + 1);  

    char  *ret = output_s;  // store the beginning pointer of buffer 

    for (size_t i=0; i < sentence_len; i++) {
        if (isspace(input_s[i]) != 0) {
            parsing_space = 1;
                        
            // If we have previously counted some non-spaces 
            if (len != 0) {
                convert_word(input_s + start_idx, len, output_s);
                output_s += len;  // move the output buffer
            }
            
            len = 0;
        } else {
            // detect transition from space to non-space character
            if (parsing_space) {
                // the previous char is a space
                len = 1;
                start_idx = i;
                parsing_space = 0;
            } else {
                len++;
            }
        }
    }
    
    // Take care of the last word
    if (len != 0) {
        convert_word(input_s + start_idx, len, output_s);
        output_s += len;  // move the output buffer
        len = 0;
    }

    // make the first char lower case
    if (sentence_len > 0 && isalpha(ret[0]) != 0) {
        ret[0] = ret[0] + 32;  // if ret[0] was a alphabetical char, it would have been uppercase because how convert_word worked
    }

    //  printf("ret = %s\n", ret);

    return ret;
}

char **camel_caser(const char *input_str)
{
    // printf("input_str = %s\n", input_str);
    int num_sentences = 0;

    // Separate input into sentences
    char **inputs = parse_inputs(input_str, &num_sentences);
    
    // Create the output data structure
    char **outputs = (char**)malloc(sizeof(char*) * (num_sentences+1));
    memset(outputs, 0, sizeof(char*) * (num_sentences+1));

    // Convert each sentence
    for (int i=0; i<num_sentences; i++) {
        outputs[i] = convert_sentence(inputs[i]);
    }

    // Free memory allocated for inputs
    destroy(inputs);
    
    return outputs;
}

void destroy(char **result) {
    char **sentence;
    
    for (sentence = result; *sentence != NULL; sentence++) {
        free(*sentence);
    }

    free(result);
}
