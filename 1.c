#define _GNU_SOURCE
#include<signal.h>
#include<stdio.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<pwd.h>
#define MAXLEN 1024
#define PIPESIZE 20

void run_cmd(char* cmdvec[MAXLEN], int cmd_cnt, int is_bg)
{
    int fcpid, cmd_idx, pipefd[PIPESIZE][2];
    int new_fd_in, old_fd_in, new_fd_out, old_fd_out;
    for(cmd_idx = 0; cmd_idx < cmd_cnt - 1; cmd_idx++)
    {
        if(pipe(pipefd[cmd_idx]) < 0)
        {
            perror("pipe");
            exit(1);
        }
    }
    for(cmd_idx = 0; cmd_idx < cmd_cnt; cmd_idx++)
    {
        int pid = fork();
        if(pid < 0)
        {
            perror("fork");
            exit(1);
        }
        if(pid)
        {
            if(!cmd_idx) fcpid = pid;
            setpgid(pid, fcpid);
            if(!cmd_idx && !is_bg) tcsetpgrp(0, fcpid);
            continue;
        }
        for(int j = 0; j < cmd_cnt - 1; j++)
        {
            if(j == cmd_idx || j == cmd_idx - 1) continue;
            close(pipefd[j][0]);
            close(pipefd[j][1]);
        }
        if(cmd_cnt != 1)
        {
            if(cmd_idx == 0)
            {
                dup2(pipefd[cmd_idx][1], 1);
                close(pipefd[cmd_idx][0]);
            }
            else if(cmd_idx == cmd_cnt - 1)
            {
                dup2(pipefd[cmd_idx - 1][0], 0);
                close(pipefd[cmd_idx - 1][1]);
            }
            else
            {
                dup2(pipefd[cmd_idx][1], 1);
                dup2(pipefd[cmd_idx - 1][0], 0);
                close(pipefd[cmd_idx][0]);
                close(pipefd[cmd_idx - 1][1]);
            }
        }
        int cmdl_cnt = 0;
        char* cmdl[MAXLEN], *cmd;
        cmd = cmdvec[cmd_idx];
        cmdl[cmdl_cnt++] = strtok(cmd, " ");
        while(cmdl[cmdl_cnt] = strtok(NULL, " "))
        {
            if(!strcmp("<", cmdl[cmdl_cnt]))
            {
                old_fd_in = dup(0);
                char* filepath = strtok(NULL, " ");
                new_fd_in = open(filepath, O_RDONLY);
                if(new_fd_in < 0)
                {
                    perror("open");
                    return;
                }
                dup2(new_fd_in, 0);
                close(new_fd_in);
            }
            else if(!strcmp(">", cmdl[cmdl_cnt]))
            {
                old_fd_out = dup(1);
                char* filepath = strtok(NULL, " ");
                new_fd_out = open(filepath, O_TRUNC | O_CREAT | O_RDWR, 0644);
                if(new_fd_out < 0)
                {
                    perror("open");
                    return;
                }
                dup2(new_fd_out, 1);
                close(new_fd_out);
            }
            else if(!strcmp("&", cmdl[cmdl_cnt]));
            else cmdl_cnt++;
        }

        execvp(cmdl[0], cmdl);
        perror("execvp");
        exit(1);
    }
    for(cmd_idx = 0; cmd_idx < cmd_cnt - 1; cmd_idx++)
    {
        close(pipefd[cmd_idx][0]);
        close(pipefd[cmd_idx][1]);
    }
    for(cmd_idx = 0; cmd_idx < cmd_cnt; cmd_idx++)
    {
        if(!is_bg) wait(NULL);
    }
    if(!is_bg) tcsetpgrp(0, getpid());
}

int split_cmd_with_pipe(char* buf, char* retvec[MAXLEN], int* is_bg)
{
    if(strstr(buf, " &")) *is_bg = 1;
    else *is_bg = 0;
    int cnt = 0;
    retvec[cnt++] = strtok(buf, "|");
    while(retvec[cnt] = strtok(NULL, "|"))
        cnt++;
    return cnt;
}

void run_shell(char* buf)
{
    int is_bg;
    char *cmd_vec[MAXLEN];

    int cmd_cnt = split_cmd_with_pipe(buf, cmd_vec, &is_bg);
    if(!cmd_cnt) return;

    if(!strncmp("cd ", cmd_vec[0], 3))
    {
        char *cd_temp = strtok(cmd_vec[0], " ");
        cd_temp = strtok(NULL, " ");
        if(chdir(cd_temp) < 0)
        perror("chdir()");
    }
    else run_cmd(cmd_vec, cmd_cnt, is_bg);
}

void handler(int sig)
{
    waitpid(-1, NULL, WNOHANG);
}

int main()
{
    signal(SIGTTOU, SIG_IGN);
    signal(SIGCHLD, handler);
    char buf[MAXLEN];
    while(1)
    {
        struct passwd *pw = getpwuid(getuid());
        char wd[MAXLEN], hostname[MAXLEN], *pwd;
        gethostname(hostname, MAXLEN);
        getcwd(wd, MAXLEN);
        pwd = wd + strlen(wd);
        for(int i = strlen(wd) - 1; *pwd != '/' && pwd != wd; pwd--);
        if(*(pwd + 1) != 0) pwd++;
        printf("%s@%s %s $ ", pw -> pw_name, hostname, pwd);

        char* buf_temp = NULL;
        long int buflen = 0;
        getline(&buf_temp, &buflen, stdin);

        if(!strcmp("\n", buf_temp)) continue;
        sscanf(buf_temp, "%[^\n]", buf);
        if(!strcmp(buf, "exit"))
        {
            printf("byebye\n");
            exit(0);
        }
        run_shell(buf);
        *buf = 0;
    }
    return 0;
}
