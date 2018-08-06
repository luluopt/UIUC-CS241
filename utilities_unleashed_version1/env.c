/**
 * Lab: Utilities Unleashed
 * CS 241 - Fall 2017
 */
#include"format.h"
#include<unistd.h>
#include<time.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>

char **strsplit(const char* str, const char* delim, size_t* numtokens) {
    // copy the original string so that we don't overwrite parts of it
    // (don't do this if you don't need to keep the old line,
    // as this is less efficient)
    char *s = strdup(str);
    // these three variables are part of a very common idiom to
    // implement a dynamically-growing array
    size_t tokens_alloc = 1;
    size_t tokens_used = 0;
    char **tokens = calloc(tokens_alloc, sizeof(char*));
    char *token, *strtok_ctx;
    for (token = strtok_r(s, delim, &strtok_ctx);
            token != NULL;
            token = strtok_r(NULL, delim, &strtok_ctx)) {
        // check if we need to allocate more space for tokens
        if (tokens_used == tokens_alloc) {
            tokens_alloc *= 2;
            tokens = realloc(tokens, tokens_alloc * sizeof(char*));
        }
        tokens[tokens_used++] = strdup(token);
    }
    // cleanup
    if (tokens_used == 0) {
        free(tokens);
        tokens = NULL;
    } else {
        tokens = realloc(tokens, tokens_used * sizeof(char*));
    }
    *numtokens = tokens_used;
    free(s);
    return tokens;
}

// 判断字符串是否是数字
int is_number(char *str){
	size_t len = strlen(str);
	for(size_t i=0;i<len;i++){
		if(isdigit(str[i])){
			continue;
		}
		else{
			return 0;
		}
	}
	return 1;
}

// 找到"--"的位置，返回-1表示没有找到位置
int find_double_lines(int argc, char *argv[]){
	int position = -1;
	for(int i=0;i<argc;i++){
		if(!strcmp(argv[i],"--")){
			position = i;
			break;
		}
	}
	return position;
}

// 构建execvp函数的参数数组
// 第三个参数为"--"的位置
char** build_array(int argc, char *argv[], int position){
	char **argv_temp = (char **)malloc((argc-position)*sizeof(char **));
	for(int j=position+1;j<argc;j++){
		argv_temp[j-(position+1)] = (char *)malloc(strlen(argv[j])+1);
		strcpy(argv_temp[j-(position+1)],argv[j]);
	}
	free(argv_temp[argc-position-1]);
	argv_temp[argc-position-1] = NULL;
	return argv_temp;
}

// 处理带引用的环境变量
void replace_ref(char **names, char **values, size_t key_num){
	for(size_t i=0;i<key_num;i++){
		if(values[i][0]=='%'){
			size_t j=0;
			for(;j<key_num&&j!=i;j++){
				if(!strcmp(names[j],values[i]+1)){
					// 将TEMP变量名设置成TZ，并把带引用的TZ置为NULL
					free(names[j]);
					names[j] = strdup(names[i]);
					free(names[i]);
					names[i] = NULL;
					free(values[i]);
					values[i] = NULL;
					break;
				}
			}
			if(j==key_num){
				// 设置为空字符串
				free(values[i]);
				values[i] = strdup("");
			}
		}
	}
}


// 释放资源
void clean_single(char **v, size_t num){
	if(v!=NULL){
		for(size_t i=0;i<num;i++){
			if(v[i]!=NULL)
				free(v[i]);
		}
		free(v);
	}
}

// 释放所有资源
void clean_all(char **names, char **values, char ***tokens_num, size_t key_num, int num, char **argv_temp, int len){
	clean_single(names,key_num);
	clean_single(values,key_num);
	clean_single(argv_temp,(size_t)len);
	if(tokens_num!=NULL)
	for(size_t i = 0;i<key_num; i++){
		if(tokens_num[i]!=NULL){
			for(int j=0;j<num;j++){
				if(tokens_num[i][j]!=NULL)
					free(tokens_num[i][j]);
			}
			free(tokens_num[i]);
		}
	}
	free(tokens_num);
}

int main(int argc, char *argv[]) {
	// 如果命令行参数小于3个(因为最少是：./env -- command)，或者大于等于3个，但是第二个参数不是"-n"也不是"--"，则使用方式出错
	if(argc<3 || (argc>=3&&strcmp(argv[1],"-n")!=0&&strcmp(argv[1],"--")!=0)){
		print_env_usage();
	}
	else if(strcmp(argv[1],"-n")==0){		// ./env -n 4 TEMP=EST5EDT,CST6CDT,MST7MDT,PST8PDT TZ=%TEMP -- date这类模式
		
		// 判断-n后面是否是数字
		if(is_number(argv[2])){
			int num = atoi(argv[2]);
			
			// 找到"--"的位置
			int position = find_double_lines(argc, argv);
			if(position == -1){
				print_env_usage();
			}
			
			// 记录共有几个key
			size_t key_num = (size_t)(position - 3);
			
			// 为环境变量名分配空间，得到所有环境变量名
			char **names = (char**)malloc(key_num*sizeof(char**));	// 记得释放空间
			// 将变量的值存储起来
			char **values = (char **)malloc(key_num*sizeof(char **));		
			for(size_t k = 0;k<key_num;k++){
				
				// 找到等号的位置
				char *pos = strstr(argv[k+3],"=");
				names[k] = (char *)malloc((pos-argv[k+3])*sizeof(char)+1);
				strncpy(names[k], argv[k+3], (int)(pos-argv[k+3]));
				values[k] = strdup(pos+1);
			}
			
			
			
			// 处理环境变量
			// 存在TZ=%TEMP这种类型的变量，查找是否有TEMP变量。如果没有，将其变量值设置为长度为0的字符串
			replace_ref(names, values, key_num);
			
			
			// 存储tokens的指针
			char ***tokens_num = (char ***)malloc(key_num*sizeof(char***));
			for(size_t k = 0; k<key_num;k++){
				char **tokens = NULL;
				size_t numtokens=0;
				if(values[k]!=NULL){
					tokens = strsplit(values[k], ",", &numtokens);
					
					// 判断分割的字符串数目是否和指定的相同，如果不同则将使用方式出错
					if((int)numtokens!=num){
						// 先释放资源再进行打印输出
						clean_all(names, values, tokens_num, key_num, num, NULL, 0);
						print_env_usage();
					}
					
				}
				else{
					free(tokens_num[k]);
				}
				tokens_num[k] = tokens;
			}
			
			// 构建参数数组
			char **argv_temp = build_array(argc, argv, position);

			int status;
			// 开启子进程
			for (int i=0;i<num;i++){
			
				pid_t child = fork();
				if(child==-1){
					print_fork_failed();
				}
				else if(child == 0){
						
						for(size_t k=0;k<key_num&&names[k]!=NULL;k++){
						
							if(setenv(names[k], tokens_num[k][i],1)==-1){		//设置环境变量失败
								print_environment_change_failed();
							}
							
							//free(tokens_num[k][i]);
						}
						
						if(execvp(argv[position+1],argv_temp)<0){
							print_exec_failed();
						}
	
				}
				else{
					
					waitpid(child , &status ,0);
					if(status != 0){
						break;
					}
				}
				
			}
			// 释放资源
			clean_all(names, values, tokens_num, key_num, num, argv_temp, argc-position);
			if(status!=0)
				return 1;
		}
		else{	
			print_env_usage();
		}
	}
	else{		// ./env -- cmd [options]模式
		int position = find_double_lines(argc, argv);
		char **argv_temp = build_array(argc, argv, position);
		
		// 开启子进程
		pid_t child = fork();
		if(child==-1){
			print_fork_failed();
		}
		else if(child == 0){
				
				if(execvp(argv[position+1],argv_temp)<0){
					print_exec_failed();
				}

		}
		else{
			int status;
			waitpid(child , &status ,0);
			if(status != 0){
				clean_all(NULL, NULL, NULL, 0, 0, argv_temp, argc-position);
				return 1;
			}
		}
	}
	

	return 0; 
}
