/**
 * Machine Problem: Shell
 * CS 241 - Fall 2017
 */
#include "format.h"
#include "shell.h"
#include "vector.h"
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>

typedef struct process {
    char *command;
    char *status;
    pid_t pid;
} process;


vector *vec;  // 存储历史命令
volatile sig_atomic_t pleaseStop = 0;

// 包含指定逻辑运算符的个数
size_t logicopt_num(char *command, char *logic){
  size_t lgc_len = strlen(logic);
  char *sub = command-lgc_len;
  size_t count = 0;
  size_t len = strlen(command);
  
  while(sub - command<(long)len && (sub = strstr(sub+lgc_len, logic))!=NULL){
    count++;
  }
  return count;
}
// 判断是否包含逻辑运算符，如果只包含一个逻辑运算符，则logic参数的值为相应的逻辑运算符字符串
int is_contain_lgcopt(char *command, char **logic){
  
  size_t and_count = logicopt_num(command, "&&");
  size_t or_count = logicopt_num(command, "||");
  size_t sep_count = logicopt_num(command, ";");
  
  if(and_count == 1 && or_count == 0 && sep_count == 0){
    *logic = strdup("&&");
  }
  else if(and_count == 0 && or_count == 1 && sep_count == 0){
    *logic = strdup("||");
  }
  else if(and_count == 0 && or_count == 0 && sep_count == 1){
    *logic = strdup(";");
  }
  
  return and_count+or_count+sep_count;
}

// 去掉字符串首尾两端空格
void trim_string(char *str)  
{  
    char *start, *end;  
    //去掉两端的空格  
    start = str;            //指向首字符  
    end = str + strlen(str) -1;     //指向最后一个字符  
    while(*start && isspace(*start))  
        start++;        //如果是空格，首地址往前移一位，如果不是，则跳过该循环  
    while(*end && isspace(*end))  
        *end-- = 0;     //如果是空格，末地址往前移一位，并赋结束符  
    strcpy(str, start);     //把首地址还给str  
}


// 加载history文件
vector *load_history(char *path){
  FILE *fp = NULL;
  fp = fopen(path, "r");
  if(fp == NULL){
    print_history_file_error();
    return NULL;
  }
  char *line;
  size_t n;
  ssize_t s;
  vector *vec = string_vector_create();
  while((s = getline(&line, &n, fp)>0)){
    if(line[strlen(line)-1]=='\n'){
      line[strlen(line)-1] = '\0';
    }
    vector_push_back(vec, line);
  }
  fclose(fp);
  return vec;
}

// 写历史文件
void write_history(char *path){
  FILE *fp = NULL;
  fp = fopen(path, "w");
  if(fp == NULL){
    print_history_file_error();
    return;
  }
  
  size_t vec_size = vector_size(vec);
  for(size_t i=0;i<vec_size;i++){
    fputs(vector_get(vec,i),fp);  
    fputs("\n",fp);
  }
  fclose(fp);
}

// 加载命令文件
vector *load_command_file(char *path){
  FILE *fp = NULL;
  fp = fopen(path, "r");
  if(fp == NULL){
    print_script_file_error();
    return NULL;
  }
  char *line;
  size_t n;
  ssize_t s;
  vector *vec = string_vector_create();
  while((s = getline(&line, &n, fp)>0)){
    if(line[strlen(line)-1]=='\n'){
      line[strlen(line)-1] = '\0';
    }
    vector_push_back(vec, line);
  }
  fclose(fp);
  return vec;
}

// 存储命令
void save_cmd(char *command){
  vector_push_back(vec, command);
}

// 显示命令
void display_cmds(){
  for(size_t i=0;i<vector_size(vec);i++){
    print_history_line(i, vector_get(vec,i));
  }
}

// 执行扩展命令  -1 表示不成功， 1 表示成功
int exec_extend_command(char *command){

  // 构建参数数组
  size_t numtokens;
  char** tokens = strsplit(command, " ", &numtokens);
  char **argv_temp = (char **)malloc(sizeof(command)*(numtokens+1));
  for(size_t i=0;i<numtokens;i++){
    argv_temp[i] = strdup(tokens[i]);
  }
  //free(argv_temp[numtokens]);
  argv_temp[numtokens] = (char *)NULL;
  
  if(numtokens==0){
    return -1;
  }
  
  pid_t child = fork();
  if(child == -1){
    print_fork_failed();
  }
  else if(child == 0){
    pid_t p = getpid();
    if(setpgid(p,getppid())==-1){
      print_setpgid_failed();
    }
    print_command_executed(p);
    int code = 0;
    if((code=execvp(tokens[0],argv_temp))==-1){
      print_exec_failed(command);
    }
    kill(p, SIGKILL);
  }
  else{
    save_cmd(command);
    int status;
    pid_t pid = waitpid(child, &status, 0);
    free_args(tokens);
    if(pid==-1){
      print_wait_failed();
      return -1;
    }
    else if(WIFEXITED(status)){
      //int low8bits = WEXITSTATUS(status);
      // printf("Process %d returned %d\n" , pid, low8bits);
      // 执行成功，子进程正常退出，返回1
      return 1;
    }
    else if(WIFSIGNALED(status)){
      //int code = WTERMSIG(status);
      //printf("code = %d\n",code);
      //print_killed_process(child, command);
      // 子进程被命令终止
      return -1;
    }
  }
  return -1;
}
int exec_command(char *command);

// 执行内建命令  -1表示不成功，1表示成功
int exec_buildin_command(char* command){
  // 去掉命令首尾两端空格W
  trim_string(command);
  if(command==NULL || strlen(command)==0){
    return -1;
  }
  size_t numtokens;
  char** tokens = strsplit(command, " ", &numtokens);
  // 判断是否是cd命令
  if(strcmp(tokens[0], "cd")==0){
    if(numtokens!=2){    // 如果参数小于2，则是无效命令
      print_invalid_command(command);
      free_args(tokens);
      return -1;
    }
    else{
      save_cmd(command);
      if(chdir(tokens[1])==-1){    // 改变目录失败，即找不到该目录
        print_no_directory(tokens[1]);
        free_args(tokens);
        return -1;
      }
      free_args(tokens);
      return 1;
    }
  }
  else if(strcmp(command,"!history")==0){
    display_cmds();
    return 1;
  }
  else if(command[0]=='!'){
    // command的长度
    size_t len = strlen(command);
    // vector大小
    size_t vec_size = vector_size(vec);
    size_t i=0;
    int result_code = -1;
    for(;i<vec_size;i++){
      size_t j=0;
      for(;j<len-1;j++){
        if(command[j+1]!=((char *)vector_get(vec,vec_size-i-1))[j]){
          break;
        }
      }
      if(j==len-1){
        // 先打印命令
        char *exe_command = (char *)vector_get(vec,vec_size-i-1);
        print_command(exe_command);
        // 执行命令
        result_code = exec_command(exe_command);
        break;
      }
    }
    if(i == vec_size){
      // 没有找到命令
      print_no_history_match();
      return -1;
    }
    return result_code;
  }
  else if(command[0]=='#'){
    // command的长度
    size_t len = strlen(command);
    // 判断#之后是否是数字
    for(size_t i=1;i < len;i++){
      if(isdigit(command[i])==0){
        print_invalid_command(command);
        return -1;
      }
    }
    // 判断是否只有#
    if(len == 1){
      print_invalid_command(command);
      return -1;
    }
    
    
    size_t vec_size = vector_size(vec);
    int cmd_num = atoi(command+1);
    int result_code = -1;
    if(cmd_num>(int)vec_size){
      print_invalid_index();
      return -1;
    }
    else{
      // 先打印命令
      char *exe_command = (char *)vector_get(vec,cmd_num);
      print_command(exe_command);
      // 执行命令
      result_code = exec_command(exe_command);
    }
    return result_code;
  }
  else{    // 内建命令无法找到，则执行扩展命令
    return exec_extend_command(command);
  }
}


void handle_signal(int signal){
  
  switch(signal){
    case SIGINT:
      if(getpid() != getpgid(0))  // 进程组长忽略Ctrl+C信号
        pleaseStop = 1;
      else
        return;
      break;
  }
}



// 执行命令
int exec_command(char *command){
  char *logic = NULL;
  int result = is_contain_lgcopt(command, &logic);
  if(result>1){
      print_invalid_command(command);
    }
    else if(result==0){
      // 直接执行命令，先执行內建命令
      return exec_buildin_command(command);
    }
    else if(result == 1){
      
      size_t numtokens;
      char **tokens;
      
      tokens = strsplit(command, logic, &numtokens);
      
      int flag = 0;  // 标记
      
      for(size_t i=0;i<numtokens;i++){
        
        int result_code = exec_buildin_command(tokens[i]);
        
        if(result_code == -1 && strcmp(logic,"&&")==0){ // 如果一个命令没有成功，并且逻辑运算符为&&，则不执行第二个
          flag = 1;
        break;
        }
        else if(result_code == 1 && strcmp(logic, "||")==0){// 如果一个命令成功，并且逻辑运算符为||，则不执行第二个
          // flag = 2;
          break;
        }
      }
      free_args(tokens);
      free(logic);
      if(flag == 1){
        return -1;
      }
    }
    return 1;
}

int shell(int argc, char *argv[]) {
    // TODO: This is the entry point for your shell.
    int oc;      // 选项字符
    int option_ch;  // 用来存储选项字符
    while((oc = getopt(argc, argv, "hf"))!=-1){
      switch(oc){
        case 'h':
          option_ch = oc;
          if(argv[optind]==NULL){    // 判断是否有文件名
            print_usage();
            return 0;
          }
          char *full_path = get_full_path(argv[optind]);
          vec = load_history(full_path);
          if(vec!=NULL){
            
        }else{
            free(full_path);
            return 0;
        }
          break;
        case 'f':
          option_ch = oc;
          if(argv[optind]==NULL){    // 判断是否有文件名
            print_usage();
            return 0;
          }
          if(vec==NULL){
            vec = string_vector_create();
        }
        
          vector *commands_file = load_command_file(get_full_path(argv[optind]));
          size_t siz = 0;
          if(commands_file!=NULL)
            siz = vector_size(commands_file);
          for(size_t i=0;i<siz;i++){
            exec_command((char *)vector_get(commands_file,i));
          }
          break;
        case '?':
          print_usage();
          return 0;
      }
    }
    
    if(vec==NULL){
      vec = string_vector_create();
    }
    signal(SIGINT,handle_signal);
    
    while(1){
      char *full_path = NULL;
      full_path = getcwd(NULL, 0);
      print_prompt(full_path,getpid());
      
      char *inputBuffer = NULL;
        size_t n = 0;
        ssize_t s = getline(&inputBuffer, &n, stdin);
        
        
        if(s>0){
          // 去掉末尾换行符
        if (inputBuffer[strlen(inputBuffer) - 1] == '\n') {
            inputBuffer[strlen(inputBuffer) - 1] = '\0';
        }
          exec_command(inputBuffer);
        }
        else{
          pleaseStop = 1;
        }
        free(full_path);
        free(inputBuffer);
      if(pleaseStop == 1){
        break;
      }
    }

   if(option_ch == 'h'){
       write_history(get_full_path(argv[optind]));
   }
   vector_destroy(vec);
    
   return 0;
}
