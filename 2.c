#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>
#include <readline/history.h>
#include <wait.h>
#include <fcntl.h>

#define MAX 128
#define DELIMS " \n\t"
#define MAX_L 256

void prompt();
void commodanalsysis(char *argv[], int number);
 int ans(char *argv[], int count);
void mycd(char *argv[]);
void mydup1(char *argv[]);
void mydup2(char *argv[]);
void mydup3(char *argv[]);
void multiple_pipelines(char *argv[], int count);
void showhistory();

int if_h=0;//&
char arr[1000]; //路径
char strpwd[MAX]; // cd 

int main() {

  read_history(NULL);
  signal(SIGINT, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  size_t linebuf_size = 0;

  while (1) {
    char *argv[MAX] = {NULL};
    char *line = NULL;
    prompt();
    if(getline(&line,&linebuf_size,stdin)<0) {
      printf("\n");
      continue;
    }
    char *commod = strtok(line,"\n");
    if (commod == NULL) {
      printf("\n");
      continue;
    }
    add_history(commod);
    write_history(NULL);
    int i = 1;
    argv[0] = strtok(commod, DELIMS);
    while (argv[i] = strtok(NULL, DELIMS))
      i++;
    commodanalsysis(argv, i);
  }
}
void prompt() {
  printf("myshell-0.1$ :");
  getcwd(arr,sizeof(arr));
  printf("\033[1m\033[34m%s\033[0m", arr);
  printf("$ :");
  fflush(stdout); 
}
void commodanalsysis(char *argv[], int number) {
  int flag = ans(argv, number);
  if(if_h==1)
    number--;
  if (flag == 1)
    mycd(argv);
  else if (strcmp(argv[0], "history") == 0)
    showhistory();
  else if (strcmp(argv[0], "exit") == 0) {
    printf("exit\n");
    exit(0);
  }
  else if (flag == 2)
    mydup1(argv);
  else if (flag == 3)
    multiple_pipelines(argv, number);
  else if (flag == 4)
    mydup2(argv);
  else if (flag == 5)
    mydup3(argv);
  else if (flag == 10) {
    pid_t pid = fork();
    if (pid < 0) {
      perror("fork");
      exit(1);
    }
    else if (pid == 0) {
      execvp(argv[0], argv);
      perror("commod");
      exit(1);
    }
    else if (pid > 0) {
      if(if_h==1) {
        if_h=0;
        printf("%d\n",pid);
        return;
      }
      waitpid(pid, NULL, 0);
    }
  }
}
void mycd(char *argv[]) {
  if (argv[1] == NULL) {
    getcwd(strpwd, sizeof(strpwd));
    chdir("/home");
  }
  else if (strcmp(argv[1], "-") == 0) {
    char strpwd1[MAX];
    getcwd(strpwd1, sizeof(strpwd));
    chdir(strpwd);
    printf("%s\n", strpwd);
    strcpy(strpwd, strpwd1);
  }
  else if (strcmp(argv[1], "~") == 0) {
    getcwd(strpwd, sizeof(strpwd));
    chdir("/home/yu666");
  }
  else {
    getcwd(strpwd, sizeof(strpwd));
    chdir(argv[1]);
  }
}
void mydup1(char *argv[]) {
  
  char *strc[MAX] = {NULL};
  int i = 0;

  while (strcmp(argv[i], ">")) {
    strc[i] = argv[i];
    i++;
  }
  int number=i;
  int flag =ans(argv, number);
  i++;
  int fdout = dup(1);
  int fd = open(argv[i], O_WRONLY | O_CREAT | O_TRUNC,6666);
  dup2(fd, 1);
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(1);
  }
  else if (pid == 0) { 
      if (flag == 3)
        multiple_pipelines(strc, number);
      else
       execvp(strc[0], strc);
  }
  else if (pid > 0) {
     if(if_h==1) {
        if_h=0;
        printf("%d\n",pid);
        return;
      }
    waitpid(pid, NULL, 0);
  }
  dup2(fdout, 1);
}
void mydup2(char *argv[]) {

  char *strc[MAX] = {NULL};
  int i = 0;

  while (strcmp(argv[i], ">>")) {
    strc[i] = argv[i];
    i++;
  }
  int number=i;
  int flag =ans(argv, number);
  i++;
  int fdout = dup(1);
  int fd = open(argv[i], O_WRONLY | O_CREAT | O_APPEND,6666);
  pid_t pid = fork();
  dup2(fd, 1);
  if (pid < 0) {
    perror("fork");
    exit(1);
  }
  else if (pid == 0) {
    if (flag == 3)
      multiple_pipelines(strc, number);
    else
      execvp(strc[0], strc);
  }
  else if (pid > 0) {
     if(if_h==1) {
        if_h=0;
        printf("%d\n",pid);
        return;
      }
    waitpid(pid, NULL, 0);
  }
  dup2(fdout, 1);
}
void mydup3(char *argv[]) {

  char *strc[MAX] = {NULL};
  int i = 0;

  while (strcmp(argv[i], "<")) {
    strc[i] = argv[i];
    i++;
  }
  i++;
  int number=i;
  int flag =ans(argv, number);
  int fdin = dup(0);
  int fd = open(argv[i], O_RDONLY,6666);
  dup2(fd, 0);
  pid_t pid = fork();
  if (pid < 0) {
     if(if_h==1) {
        if_h=0;
        printf("%d\n",pid);
        return;
      }
    perror("fork");
    exit(1);
  }
  else if (pid == 0) {
    if (flag == 3)
      multiple_pipelines(strc, number);
    else
      execvp(strc[0], strc);
  }
  else if (pid > 0)
    waitpid(pid, NULL, 0);
  dup2(fdin, 0);
}
void multiple_pipelines(char *argv[], int count) {

  pid_t pid;
  int ret[10];
  int number=0;

  for(int i=0;i<count;i++) {
    if(!strcmp(argv[i],"|"))
      ret[number++]=i;
  }
  int cmd_count=number+1;
  char* cmd[cmd_count][10];
  for(int i=0;i<cmd_count;i++) {
    if(i==0) {
      int n=0;
      for(int j=0;j<ret[i];j++)
        cmd[i][n++]=argv[j];
      cmd[i][n]=NULL;
    }
    else if(i==number) {
      int n=0;
      for(int j=ret[i-1]+1;j<count;j++)
        cmd[i][n++]=argv[j];
      cmd[i][n]=NULL;
    }
    else {
      int n=0;
      for(int j=ret[i-1]+1;j<ret[i];j++)
        cmd[i][n++]=argv[j];
      cmd[i][n]=NULL;
    }
  }
  int fd[number][2];  
  for(int i=0;i<number;i++)
    pipe(fd[i]);
  int i=0;
  for(i=0;i<cmd_count;i++) {
    pid=fork();
    if(pid==0)
    break;
  }
  if(pid==0) {
    if(number) {
      if(i==0) {
        dup2(fd[0][1],1); 
        close(fd[0][0]);
        for(int j=1;j<number;j++) {
          close(fd[j][1]);
          close(fd[j][0]);
        }
      }
      else if(i==number) {
        dup2(fd[i-1][0],0);
        close(fd[i-1][1]);
        for(int j=0;j<number-1;j++) {
          close(fd[j][1]);
          close(fd[j][0]);
        }
      }
      else {
        dup2(fd[i-1][0],0);
        close(fd[i-1][1]);
        dup2(fd[i][1],1);
        close(fd[i][0]);
        for(int j=0;j<number;j++) {
             if(j!=i&&j!=(i-1)) {
               close(fd[j][0]);
               close(fd[j][1]);
             }
        }
      }
    }
    execvp(cmd[i][0],cmd[i]);
    perror("execvp");
    exit(1);
  }
    for(i=0;i<number;i++) {
        close(fd[i][0]);
        close(fd[i][1]);
    }
     if(if_h==1) {
        if_h=0;
        printf("%d\n",pid);
        return;
      }
  for(int j=0;j<cmd_count;j++)
  wait(NULL);
}
int ans(char *argv[], int count) {

  int flag = 10, i;

  if (argv[0] == NULL)
    return 0;
  if (strcmp(argv[0], "cd") == 0)
    flag = 1;
  for (i = 0; i < count; i++) {
    if (strcmp(argv[i], ">") == 0)
      flag = 2;
    if (strcmp(argv[i], "|") == 0)
      flag = 3;
    if (strcmp(argv[i], ">>") == 0)
      flag = 4;
    if (strcmp(argv[i], "<") == 0)
      flag = 5;
    if (strcmp(argv[i], "<<") == 0)
      flag = 6;
    if (strcmp(argv[i], "&") == 0) {
      if_h = 1;
      argv[i]=NULL;
    }
  }
  return flag;
}
void showhistory() {

  int i = 0;
  HIST_ENTRY **his;

  his = history_list();
  while (his[i] != NULL)
    printf("%-3d   %s\n", i, his[i++]->line);
}