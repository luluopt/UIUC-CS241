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
    char *command;
    char *status;
    pid_t pid;

    struct process * next;

} process;

typedef process * pprocess;

static pprocess processlist = NULL; // static variable to store the pointer to the process list.
static size_t processnum = 0;

// initialize the process list
pprocess initialize(pid_t pid){

  pprocess head = (pprocess)malloc(sizeof(process));

  head ->command = (char * ) malloc(strlen("super_shell"));


  strcpy(head->command,"super_shell");


  head->pid = pid;


  head->status = (char *)malloc(sizeof(STATUS_RUNNING));

  strcpy(head->status,STATUS_RUNNING);

  head->next = NULL;

  //processnum++;
  return head;

} // initialize

// add a process to the process list
int addprocess(pprocess new_process, pprocess * head,size_t * num){


  pprocess iterator  = processlist;



//  iterator -> next = new_process;
  //
  while (iterator!= NULL) {

     if(iterator->pid == new_process->pid){

       break;

     }
   else {

        iterator = iterator -> next;

     }

  }// while

  //  if(iterator == NULL){
  //
  //   new_process -> next = *head;
  //
  //   *head   = new_process;
  //
  //   (*num) ++;
  //   return 1;
  //
  // }
  // else{
  //   return -1;
  // }
if(iterator==NULL){

  iterator = processlist;
  while (iterator->next!=NULL) {
    iterator = iterator->next;
  }
  iterator->next = new_process;
  printf("process id %d\n",new_process->pid );


  size_t temp = *num;
  temp ++;

  *num = temp;

    return 1;
}
else return -1;




}//addprocess

//delete a process from the process list
// return 1 for success and -1 for fails
int deleteprocess(pid_t pid, pprocess head){

  pprocess iterator = head;


  if (iterator -> pid ==pid ) {
    head = head ->next;

    free(iterator);

    return 1;
  }
  else{

  pprocess predecessor = head;
  iterator = iterator -> next;
  while (iterator != NULL) {
    if (iterator->pid == pid) break;
    else {

      predecessor = iterator;
      iterator = iterator -> next;

    }
  }//  while (iterator != NULL) {

  if (iterator){ // if the iterator is not NULL, that means the node with pid is found,
    predecessor -> next  = iterator ->next;
    if(iterator->command)free(iterator->command);
    if(iterator->status) free(iterator ->status);
    free(iterator);
    return 1;
  }//if (iterator)
  else { // cannot found the node with the requested pid.
    return -1;
  }//if (iterator)



}//if (iterator -> pid ==pid )
} // deleteprocess


//delete all the nodes in the list.

int freeallnodes(pprocess head)
{
  pprocess iterator = head;
  pprocess predecessor = NULL;

  while(iterator != NULL)
  {
    predecessor = iterator;
    iterator = iterator -> next;

    if(predecessor->command)free(predecessor->command);
    if(predecessor->status) free(predecessor ->status);
    free(predecessor);
  }
  return 1;
} //freeallnodes



// print the process list
int printprocess()
{
  printf("%zu\n",processnum );

  if(processlist){
    pprocess iterator = processlist;
    while(iterator != NULL)
    {
      print_process_info(iterator->status,iterator->pid,iterator ->command);

      iterator = iterator ->next;
      if(iterator!= NULL) printf("%d\n", iterator ->pid );

    }
    return 1;


  }//  if(head){
  else{
    print_exec_failed("ps");
    return -1;

  }


} // printprocess


int stopprocess(pid_t pid,pprocess head)
{
  pprocess iterator= head;
  while(iterator != NULL){
    if (iterator->pid!= pid)iterator =iterator -> next;
    else break;
  }//while


  if(iterator){
    if (strcmp(iterator ->status,STATUS_RUNNING) == 0)
    {
      free(iterator->status);
      iterator -> status = (char * )malloc(sizeof(STATUS_STOPPED));
      strcpy(iterator ->status,STATUS_STOPPED);
      return 1;
    }
    else return 0;
  }
  else return 0;

}

int startprocess(pid_t pid,pprocess head)
{
  pprocess iterator= head;
  while(iterator != NULL){
    if (iterator->pid!= pid)iterator =iterator -> next;
    else break;
  }//while


  if(iterator){
    if (strcmp(iterator ->status,STATUS_STOPPED) == 0)
    {
      free(iterator->status);
      iterator -> status = (char * )malloc(sizeof(STATUS_RUNNING));
      strcpy(iterator ->status,STATUS_RUNNING);
      return 1;
    }
    else return 0;
  }
  else return 0;

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

        if(kill(cmd_num, SIGCONT)==0){
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
        else{
          strcat(final_expression,expressions_right[i]);
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
