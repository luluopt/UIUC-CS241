#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <time.h>
#include "camelCaser.h"


/*
char * myStrtok(char *s, const char *delim, int * num);
char * concatStringArray(char ** array);
char ** getWordsFromSentence(char * sentence);
char * sentence2CamelCase(char * sentence);
char **camel_caser(const char *input_str);
*/
unsigned int rand_interval(unsigned int min, unsigned int max);
/*
char * myStrtok(char *s, const char *delim, int * num) {
    static char *lasts;
    register int ch;
    if (s == 0)
	s = lasts;
    do {
	if ((ch = *s++) == '\0')
	    return 0;
	// when the current character is a delim, let's add an empty string to the pointer array
	if(strchr(delim, ch)) {
		*num = *num + 1;
	}
    } while (strchr(delim, ch));
    --s;
    lasts = s + strcspn(s, delim);
    if (*lasts != 0)
	*lasts++ = 0;
    return s;
}
char * concatStringArray(char ** array) {
	// calculate the length needed to be allocated
	int len = 0;
	int i = 0;
	char ** copy = array;
	while(*copy) {
		len = len + strlen(*copy);
		copy++;
	}
	char * camel = (char*)malloc(sizeof(char)*len + 1);
	memset(camel, 0, sizeof(char)*len + 1);
	copy = array;
	while(*copy) {
		strcat(camel, *copy);
		copy++;
	}
	return camel;
}
char ** getWordsFromSentence(char * sentence) {
	int len = strlen(sentence);
	char ** words = malloc(sizeof(char*) * len + sizeof(char*));
	memset(words, 0, sizeof(char*) * len + sizeof(char*));
	char test[] = "Hello how are you"; 	
	int curr = 0;
	char * space = (char*)malloc(sizeof(char)*128);					// freed	
	memset(space, 0, sizeof(char)*128);
	int ch;
	for(ch = 0; ch <= 127; ch++) {
		if(isspace((char)ch)) {
			space[curr++] = (char)ch;
		}
	}
	space[curr] = '\0';
	// test
	curr = 0;
	char * pch;
	pch = strtok (sentence, space);
	while (pch != NULL)
	{
		int lenSplit = strlen(pch);
		char * newStr = (char*)malloc(1 + sizeof(char)*lenSplit);
		memset(newStr, 0, 1 + sizeof(char)*lenSplit);
		strcpy(newStr, pch);
		words[curr++] = newStr;
		pch = strtok (NULL, space);
	}
	return words;
}
char * sentence2CamelCase(char * sentence) {
	char ** words = getWordsFromSentence(sentence);
	char ** copy = words;
	int i = 0;
	int k = 0;
	
	while(*words) {
		// for the first word, first letter has to be lowercase
		// for latter word, first letter is Upper Case
		char * str = *words;
		int lenStr = strlen(str);
		if(words == copy) {
			// for the first word, all letters in lowercase
			for(k = 0; k < lenStr; k++) {
				if(isalpha(str[k]))
					str[k] = tolower(str[k]);
			}
		}
		else {
			int markUpper = 0;
			for(k = 0; k < lenStr; k++) {
				if(markUpper == 0) {
					if(isalpha(str[k])) {
						str[k] = toupper(str[k]);
						markUpper = 1;
					}
				}
				else if (isalpha(str[k])) {
					str[k] = tolower(str[k]);
				}
			}
		}
		words++;
		
	}
	// concatenate all strings
	// calculate the length needed for the resulting string
	char * result = concatStringArray(copy);
	return result;
}
char **mycamel_caser(const char *input_str) {
      if(input_str == NULL) {
 	  return NULL;
        }
	// extract n sentences
	// for each sentences, turn it into camelCase
	// get all sentences seperated by puncts
	int size = (strlen(input_str)+1);
	// if sentence length is 0 then return an empty pointer array terminated with NULL
	if(size == 1) {
		char ** zerolencase = (char**)malloc(sizeof(char*));
		memset(zerolencase, 0, 4);
		return zerolencase;	
	}
	char * temp = NULL;
	char *sentence = (char*)malloc(sizeof(char)*size);				 // freed
	memset(sentence, 0, sizeof(char)*size);		
	char ** ret_arr = (char**)malloc(sizeof(char*)*size + sizeof(char*));	// freed once, return the second malloc ones to main()
	memset(ret_arr, 0, sizeof(char*)*size + sizeof(char*));
	
	char * newStr = NULL;
	int curr = 0;
	char * delimit = (char*)malloc(sizeof(char)*128);					// freed	
	memset(delimit, 0, sizeof(char)*128);
	int ch;
	for(ch = 0; ch <= 127; ch++) {
		if(ispunct((char)ch)) {
			delimit[curr++] = (char)ch;
		}
	}
	delimit[curr] = '\0';
	curr = 0;
	strcpy(sentence, input_str);
	char * pch = NULL;
	char ** copy = ret_arr;
	int num = 0;
	pch = myStrtok(sentence, delimit, &num);
	while(num != 0) {
		ret_arr[curr] = malloc(1);
		ret_arr[curr][0] = '\0';
		curr = curr + 1;
		num--;
	}
	while (pch != NULL)
	{
	    int lenSplit = strlen(pch);
	    newStr = (char*)malloc(1 + sizeof(char)*lenSplit);				// freed
	    memset(newStr,0, 1 + sizeof(char)*lenSplit);
	    strcpy(newStr, pch);
	    num = 0;
	    pch = myStrtok (NULL, delimit, &num);
	    if(pch != NULL) {
		ret_arr[curr++] = newStr;
	     }
	    else {
		char c = input_str[size-2];
		if(ispunct(c))
	    		ret_arr[curr++] = newStr;
		else
			free(newStr);
	     }
	    while(num != 0) {
		ret_arr[curr] = malloc(1);
		ret_arr[curr][0] = '\0';
		curr = curr + 1;
		num--;
  	     }
	}
	// for each ret_arr element, replace with newStr
	while(*copy) {
		int length = strlen(*copy);
		temp = (char*)malloc(sizeof(char)*length + 1);				// freed
		memset(temp, 0, sizeof(char)*length + 1);
		strcpy(temp, *copy);
		free(*copy);
		*copy = sentence2CamelCase(temp);						// return value is on heap, but doesn't need to be freed
		free(temp);
		copy++;
	}
	
	free(sentence);
	free(delimit);
	return ret_arr;
}
void print_output(char **output){
    printf("Got the following:\n");
    if(output){
        char ** line = output;
        while(*line){
            printf("\t'%s'\n", *line);
            line++;
        }
    } else{
        printf("NULL POINTER!\n");
    }
    printf("-----------------------------------------------------------------------------------------\n");
}
void print_input(char * input){
    printf("testing: \n\t'%s'\n\n", input);
}
*/
char* splitt(char* x);
void punc(char* x);

char * marrie(char* s, const char* delim,char ** array, int * count);/////////////////////////


char **marrie_caser(const char *input_str) {
    if(input_str == NULL)
      return NULL;
    if(strcmp("", input_str)==0) {
    	char ** ret = malloc(4);
    	*ret = NULL;
    	return ret;	
     }
     

    char** result = malloc((1+strlen(input_str))*sizeof(char*));// no need to return
    char** copy = result;
    char** copy2 = result;
    memset(result,0,(1+strlen(input_str))*sizeof(char*));
    char *punctuation = malloc(sizeof(char)*128);//freed
    memset(punctuation,0,sizeof(char)*128);
    punc(punctuation);
    
    char *p = strdup(input_str);//freed
    int xx =0;
    
    int count = 0;
    char* m = marrie(p,punctuation,result, &count);
    
   while (m != NULL)
  {

    *(result++)=strdup(m);//free later!! save strtok to a heap string
    count = 0;
    m = marrie (NULL, punctuation,result, &count);
    result = result + count;
  }
  char c = input_str[strlen(input_str)-1];
  if(!ispunct(c))  
  {
  	char *r=*(result-1);
   	*(result-1)=0;
   	free(r);
  }
  
     while(*copy)
     {
        char* lalala = *copy;
        *copy = splitt(*copy);
        free(lalala);
        copy++;
     }//strtok again for each cell
      
      
     free(punctuation);
     free(p);
      
    return copy2;
}


char* splitt(char* x)
{
    if(strcmp(x, "")==0) {
        return strdup("");
    }
    char** temp=malloc((1+strlen(x))*sizeof(char*));//allocate numbe of words array  free later!!
    memset(temp,0,(1+strlen(x))*sizeof(char*));

    
    //temp[strlen(x)]=NULL;//set null pointer;
    char ** s = temp;
    char *space=" \t\n\v\f\r";
    char* l = strtok(x,space);//l = "hello"
    int count =0;
   while (l != NULL)
  {  
    count++;//number of seperate words
    *(temp++)=l;
    l = strtok (NULL, " ");
  }
  
   *temp = NULL;//terminnate the last pointer
    
    // 到目前为止，count =2; s[0] ="hello", s[1]="world“，s[2]及以后都为null
   
   char* a = s[0];//tolower the first word
    
   while(*a!=0)
   {
    *a=tolower(*a);
     a++;
   }
        
   
    if(count==1) 
    {
      char * final = strdup(s[0]);//free later!!
      free(s);
      return final; 
    
    }//finish:only one word
     
    else
    {
        int z;
       for(z=1;z<count;z++)
       {
            //set all the length to lower first
             char* b = s[z];
             char* bb=s[z];
             while(*b!=0)
            {
                *b=tolower(*b);
                 b++;
            }
            while(*bb!=0)
            {
               if(isalpha(*bb))
               {
                   *bb=(char)toupper(*bb);//upper first letter
                   break;
               }
               else 
               bb++;
               
            }
            
    
       }
       
            char* final2=malloc((2+strlen(x))*sizeof(char*));//free larer!!
            memset(final2,0,(2+strlen(x))*sizeof(char*));

        
            int k;
            for(k=0;k<count;k++)
               strcat(final2, s[k]);
               
         
       free(s);
       return final2;
     }
        

     

}


void punc(char* x)
{
    int i=0;
    for(i=0;i<=127;i++)
    {
        if(ispunct((char)i))
        *(x++)=(char)i;
    }
}  // success. get all the punctuations






char * marrie(char * s, const char * delim, char ** array, int * count)
{
    static char *lasts;
    register int ch;
    
    if (s == 0)
        s = lasts;
    do {
        if((ch = *s++) == '\0')
            return 0;
        if(strchr(delim, ch)){
            *(array++)= strdup("");
            *count = *count + 1;
        }
        
    } while (strchr(delim, ch));
    --s;
    lasts = s + strcspn(s, delim);
    if (*lasts != 0)
        *lasts++ = 0;
    return s;
}
unsigned int rand_interval(unsigned int min, unsigned int max)
{
    int r;
    const unsigned int range = 1 + max - min;
    const unsigned int buckets = RAND_MAX / range;
    const unsigned int limit = buckets * range;


    do
    {
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}


int randomSmallTest(char ** (* camelCaser)(const char *), char ** (* answer)(const char *)) {
	int testlen = 30;
	srand(time(NULL));
	char sentence[testlen+1];
	char ch;
	// generate a sentence mixed with all 127 possible chars, insert space in between (length = 30)
	int i;
	for(int i = 0; i < testlen; i++) {
		unsigned int addSpace = rand_interval(1,20);
		if (addSpace < 5) {
			switch(addSpace) {
				case 1:
					ch = ' ';
					break;
				case 2:
					ch = '\t';
					break;
				case 3:
					ch = '\n';
					break;
				case 4:
					ch = '\r';
					break;
				case 5:
					ch = '\f';
					break;	
				default:
					break;
			}
		}
		else {
			ch = (char)rand_interval(0, 127);
		}
		sentence[i] = ch;
	}
	sentence[testlen] = '\0';
	// call your implementation/reference to generate a pointer array
	char ** myResult = camelCaser(sentence);
	char ** correctResult = answer(sentence);
	char ** saveMyResult = myResult;
	// diff the pointer array
	i = 0;
	while(*myResult) {
		if(strcmp(*myResult, correctResult[i++]) != 0) {
			return 0;
		}
		myResult++;
	}

	print_input(sentence);
	printf("My Result:\n");
	print_output(saveMyResult);
	printf("Correct Result:\n");
	print_output(correctResult);

	int mySize = sizeof(myResult) / sizeof(char*);
	int correctSize = sizeof(correctResult) / sizeof(char*);
	if(mySize != correctSize) {
		return 0;	
	}
	return 1;
}

void print_output(char **output){
    printf("Got the following:\n");
    if(output){
        char ** line = output;
        while(*line){
            printf("\t'%s'\n", *line);
            line++;
        }
    } else{
        printf("NULL POINTER!\n");
    }
    printf("-----------------------------------------------------------------------------------------\n");
}

void print_input(char * input){
    printf("testing: \n\t'%s'\n\n", input);
}

int main() {

	int i;
	for(i = 0; i < 5; i++) {
		printf("%s\n", (randomSmallTest(marrie_caser, camel_caser) == 1)?"succeed":"fail");
	}
    return 0;
}