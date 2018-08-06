/**
 * Machine Problem: Super Shell
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
#include <stdlib.h>

typedef struct process {
  char* command;
  char* status;
  pid_t pid;
} process;

static vector* processList = NULL;
static vector* historyCommands = NULL;
int exec_shell(char* command);

#define ERROR_INVALID_COMMAND -10
#define ERROR_NO_HISTORY -11
#define ERROR_INVALID_INDEX -12
#define ERROR_NO_DIR -13
#define ERROR_NO_PROCESS -14
#define EXIT_CODE -99

void commandOutput(int statusCode, char* command) {
  switch (statusCode) {
    case ERROR_NO_PROCESS:
    case EXIT_CODE:
    case 0:
    // no output
    break;
    case ERROR_INVALID_COMMAND:
    print_invalid_command(command);
    break;
    case ERROR_NO_HISTORY:
    print_no_history_match();
    break;
    case ERROR_INVALID_INDEX:
    print_invalid_index();
    break;
    case ERROR_NO_DIR:
    print_no_directory(command+3);
    break;
    default:
    print_exec_failed(command);
    break;
  }
}

bool isVarNameChar(char ch) {
  return (('a' <= ch && ch <= 'z') ||
              ('A' <= ch && ch <= 'Z') ||
              ('0' <= ch && ch <= '9') ||
              ch == '_');
}

char* replaceWithEnv(char* value) {
  char* ret = malloc(sizeof(char) * 256);
  int size = strlen(value);
  int printPos = 0;
  int varStart = 0;
  bool var = false;
  for (int i = 0; i <= size; i++) {
    char v = value[i];
    if (i == size) v = '$';
    if (var) {
      if (isVarNameChar(v)) {
        continue;
      } else {
        char varName[i - varStart + 3];
        for (int j = varStart; j < i; j++) {
          varName[j - varStart] = value[j];
          varName[j - varStart + 1] = '\0';
        }
        char * env = getenv(varName);
        int envSize = 0;
        if(env) envSize = strlen(env);
        for (int j = 0; j<envSize; j++) {
          ret[printPos] = env[j];
          ret[printPos+1] = '\0';
          printPos++;
        }
        var = false;
      }
    }
    if (v == '$') {
      var = true;
      varStart = i+1;
      continue;
    }
    else {
      ret[printPos] = v;
      ret[printPos+1] = '\0';
      printPos++;
    }
  }
  return ret;
}

process* findProcess(pid_t pid) {
  process* p_ptr;
  for (int i = 0 ; i < (int) vector_size(processList); i++) {
    p_ptr = (process*)vector_get(processList, i);
    if (p_ptr->pid == pid) {
      return p_ptr;
    }
  }
  print_no_process_found(pid);
  return NULL;
}


void removeProcess(pid_t pid) {
  process* p_ptr;
  for (int i = 0 ; i < (int) vector_size(processList); i++) {
    p_ptr = (process*)vector_get(processList, i);
    if (p_ptr->pid == pid) {
      vector_erase(processList, i);
    }
  }
}


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

void load_history(char *path, vector* commandVector){
  FILE *fp = NULL;
  fp = fopen(path, "r");
  if(fp == NULL){
    print_history_file_error();
    return;
  }
  char *line = (char*) malloc(256*sizeof(char));
  size_t n;
  ssize_t s;
  while((s = getline(&line, &n, fp)>0)){
    if(line[strlen(line)-1]=='\n'){
      line[strlen(line)-1] = '\0';
    }
    vector_push_back(commandVector, line);
  }
  fclose(fp);
}

void write_history(char *path, vector* commandVector){
  FILE *fp = NULL;
  fp = fopen(path, "w");
  if(fp == NULL){
    print_history_file_error();
    return;
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
  pid_t p = 0;
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
  else if(child == 0){// This is executed by the child process
    p = getpid();
    if(setpgid(p,getppid())==-1){
      print_setpgid_failed();
    }

    bool display = false;
     if(!display){
       printprocess();
       display = true;
     }
    print_command_executed(p);
    int code = 0;
    if((code=execvp(tokens[0],argv_temp))==-1){
      print_exec_failed(command);
    }
    kill(p, SIGKILL);
  //  deleteprocess(p,processlist);
  }//  else if(child == 0){
  else {  // parents process
    save_cmd(command);

    int status;
    pid_t pid = waitpid(child, &status, 0);
    free_args(tokens);
    if(pid==-1){
      print_wait_failed();
      return -1;
    }//if(pid==-1){
    else if(WIFEXITED(status)){
      //int low8bits = WEXITSTATUS(status);
      // printf("Process %d returned %d\n" , pid, low8bits);
      // 执行成功，子进程正常退出，返回1
      pprocess new_process = NULL;
      new_process = (pprocess)malloc(sizeof(process));
      new_process ->pid =p;
      new_process -> status = (char *)malloc(sizeof(STATUS_RUNNING));
      strcpy(new_process -> status, STATUS_RUNNING);
      new_process -> command = (char *)malloc(strlen(command));
      strcpy(new_process ->command, command);
      addprocess(new_process,&processlist,&processnum);
      return 1;
    } //if(pid==-1){ else if(WIFEXITED(status)){
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

/*



pprocess new_process = NULL;
new_process = (pprocess)malloc(sizeof(process));
new_process ->pid =p;
new_process -> status = (char *)malloc(sizeof(STATUS_RUNNING));
strcpy(new_process -> status, STATUS_RUNNING);
new_process -> command = (char *)malloc(strlen(command));
strcpy(new_process ->command, command);
addprocess(new_process,&processlist,&processnum);


pprocess new_process = NULL;
new_process = (pprocess)malloc(sizeof(process));
new_process ->pid =p;
new_process -> status = (char *)malloc(sizeof(STATUS_RUNNING));
strcpy(new_process -> status, STATUS_RUNNING);
new_process -> command = (char *)malloc(strlen(command));
strcpy(new_process ->command, command);
addprocess(new_process,&processlist,&processnum);
*/

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
  }// else if(strcmp(command,"!history")==0)
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
  else if(strcmp(command,"ps")==0){
    return printprocess();
  } //ps
  else if (strcmp(tokens[0], "kill")==0){
    size_t len = strlen(tokens[1]);
    for(size_t i=0;i < len;i++){
      if(isdigit(command[i])==0){
        print_invalid_command(command);
        free_args(tokens);
        return -1;
      }
      if (chdir(tokens[start + 1])) {
        return ERROR_NO_DIR;
      }
      return 0;
    }
    if(len == 0){
      print_invalid_command(command);
      free_args(tokens);
      return -1;
    }
    else{
      save_cmd(command);
      pid_t cmd_num = (pid_t)atoi(tokens[1]);
      if(deleteprocess(cmd_num,processlist) ==1){
        if(kill(cmd_num, SIGTERM)==0){
          print_killed_process(cmd_num,command);
          return 1;
        }
      }
      if (len == 1) {
        return ERROR_INVALID_COMMAND;
      }
      int cmd_num = atoi(tokens[start]+1);
      if (cmd_num >= (int)vector_size(historyCommands)) {
        return ERROR_INVALID_INDEX;
      }
      char *exe_command = (char *)vector_get(historyCommands,cmd_num);
      print_command(exe_command);
      return exec_shell(exe_command);
    } else if (strcmp(tokens[start], "ps") == 0) {
      // implement ps
      for (int i = 0; i < (int)vector_size(processList); i++) {
        process* p = vector_get(processList, i);
        int stat_val;
        pid_t status = waitpid(p->pid, &stat_val, WNOHANG);
        if (status == 0 || getpid() == p->pid) {
          print_process_info(p->status,p->pid,p->command);
        } else {
          vector_erase(processList, i);
          i--;
        }
      }//if(deleteprocess(cmd_num,processlist) ==1)
      else{
        print_no_process_found(cmd_num);
        return -1;
      }
    }//if(len == 0)
  } //kill
  else if (strcmp(tokens[0], "stop")==0){
    size_t len = strlen(tokens[1]);
    for(size_t i=0;i < len;i++){
      if(isdigit(command[i])==0){
        print_invalid_command(command);
        free_args(tokens);
        return -1;
      }
    }
    if(len == 0){
      print_invalid_command(command);
      free_args(tokens);
      return -1;
    }
    else{
      save_cmd(command);
      pid_t cmd_num = (pid_t)atoi(tokens[1]);
      if(stopprocess(cmd_num,processlist) ==1){
        if(kill(cmd_num, SIGTSTP)==0){
          print_stopped_process(cmd_num,command);
          return 1;
        }
        else{
          print_exec_failed(command);
          return -1;
        }
      }//if(deleteprocess(cmd_num,processlist) ==1)
      else{
        print_no_process_found(cmd_num);
        return -1;
      }
    }//if(len == 0)
  } // stop
  else if (strcmp(tokens[0], "cont")==0){
    size_t len = strlen(tokens[1]);

    for(size_t i=0;i < len;i++){
      if(isdigit(command[i])==0){
        print_invalid_command(command);
        free_args(tokens);
        return -1;
      }
    } else {
      end++;
    }
  }
  if (end - start > 0) {
    _status = exec_command(tokens, start, end);
  }
  return _status;
}


int exec_shell(char* command) {
  char* old_command = command;
  command = replaceWithEnv(old_command);
  size_t numtokens;
  char ** tokens = strsplit(command, " ", &numtokens);
  int statusCode = 0;
  if (numtokens > 0) { // if the command is empty, no execution
    // parse commend into tokens
    bool async = false;
    if (strcmp(tokens[numtokens - 1], "&")==0) {
      // Async, no need to wait
      async = true;
    }

    if (async) {
      // for a new child process
      pid_t child = fork();
      if (child == -1) {
        print_fork_failed();
      } else if (child == 0) {
        // child process
        free(tokens[numtokens - 1]);
        tokens[numtokens - 1] = NULL;
        // token parsing finished
        pid_t p = getpid();

        print_command_executed(p);
        statusCode = exec_shell_command(tokens);
        commandOutput(statusCode, command);
        // command finish, end the child
        for (size_t i = 0; i < vector_size(processList); i++) {
          process* p_in_list = (process*)vector_get(processList, i);
          if (p_in_list->pid == p) {
            free(vector_get(processList, i));
            vector_erase(processList, i);
            break;
          }
        }
        free_args(tokens);
        kill(p, SIGTERM);
      } else {
        pid_t p = child;
        process* pro = malloc(sizeof(process));
        pro->pid = p;
        pro->status = (char *)malloc(sizeof(STATUS_RUNNING));
        strcpy(pro->status, STATUS_RUNNING);
        pro->command = (char *)malloc(strlen(command));
        strcpy(pro->command, command);
        vector_push_back(processList, pro);
        return statusCode;
      }
    } else {
      print_command_executed(getpid());
      statusCode = exec_shell_command(tokens);
      commandOutput(statusCode, command);
      if(statusCode != EXIT_CODE) {
        save_cmd(command, historyCommands);
      }
    }
  } else {
    free_args(tokens);
  }
  return statusCode;
}

    }//if(len == 0)

  } // cont
  else if (strcmp(command,"exit")==0){
    return 0;
  } // exit
  else if (strcmp(tokens[0],"export") == 0){
    // Firstly, split the =
    size_t num_sides;
    char ** expressions = strsplit(tokens[1], "=", &num_sides);

    if(num_sides==2){
      // secondly, split the : in the right expressions
      size_t num_tokens_right;
      char ** expressions_right = strsplit(expressions[1], ":", &num_tokens_right);
      char final_expression[100];
      memset(final_expression,0x00,100);
      char temp_variable_name[30];
      for(size_t i = 0 ; i < num_tokens_right;i ++)
      {
        if(expressions_right[i][0]=='$') {
          memset(temp_variable_name,0x00,30);
          strcpy(temp_variable_name,expressions_right[i]+1);
          strcat(final_expression,getenv(temp_variable_name));
        }
        char *full_path = get_full_path(argv[optind]);
        load_history(full_path, historyCommands);
        break;
      case 'f':
        if(argv[optind]==NULL){ // check file name
          print_usage();
          return 0;
        }
        if(i != num_tokens_right -1) strcat(final_expression,":");
      }
      //printf("%s",expressions[0]);
      //printf("%s",final_expression);
      setenv(expressions[0],final_expression,1);
      return 1;
    }
    else return -1;
  }// export
  else if (strcmp(tokens[0], "echo") == 0) {
    for(size_t i = 1; i < numtokens; i++) {
      if (tokens[i][0] == '$') {
        if (getenv(&tokens[1][1])) {
          printf("%s ", getenv(&tokens[i][1]));
        }
        else {
          printf(" ");
        }
      }
      else {
        printf("%s ", &tokens[i][0]);
      }
    }
    printf("\n");
    return 1;
  }
  else{    // 内建命令无法找到，则执行扩展命令
    return exec_extend_command(command);
  }
}

int shell(int argc, char *argv[]) {
  processList = shallow_vector_create();
  historyCommands = string_vector_create();

  pid_t pid = getpid();
  process* pro = malloc(sizeof(process));
  pro->pid = pid;
  pro->status = (char *)malloc(sizeof(STATUS_RUNNING));
  strcpy(pro->status, STATUS_RUNNING);
  pro->command = (char *)malloc(strlen("super_shell")+1);
  strcpy(pro->command, "super_shell");
  vector_push_back(processList, pro);

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
int exec_command(char *command) {
  char *logic = NULL;
  int result = is_contain_lgcopt(command, &logic);
  if(result > 1){
    print_invalid_command(command);
  }
  else if(result == 0){
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
      // 如果一个命令没有成功，并且逻辑运算符为&&，则不执行第二个
      if(result_code == -1 && strcmp(logic,"&&")==0){ 
        flag = 1;
        break;
      }
      // 如果一个命令成功，并且逻辑运算符为||，则不执行第二个
      else if(result_code == 1 && strcmp(logic, "||")==0){
        // flag = 2;
        break;
      }
      else if(result_code == 0) {
        return 0;
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

  processlist = initialize(getpid());

  while(1){
    char *full_path = NULL;
    full_path = getcwd(NULL, 0);
    print_prompt(full_path,getpid());

    char *inputBuffer = NULL;
    size_t n = 0;
    ssize_t s = getline(&inputBuffer, &n, stdin);

    if(s>0){
      // 去掉末尾换行符
      if(inputBuffer[strlen(inputBuffer) - 1] == '\n') {
        inputBuffer[strlen(inputBuffer) - 1] = '\0';
      }
      if(exec_command(inputBuffer) == 0) {
        pleaseStop = 1;
      }
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
 freeallnodes(processlist);

 return 0;
}
