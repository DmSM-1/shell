#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <pwd.h>


#define COPACITY 1024
#define WORD 256
#define DUMP dump(input_buf, command_buf, command_count, err_flag)


int read_to(char* input_buf){
    char ch = 0;
    for(int i = 0; i < COPACITY; i++)
        input_buf[i] = 0;
    for(int i = 0; i < COPACITY; i++){
        ch = getchar();
        if (ch == '\n' || ch == EOF)
            break;
        input_buf[i] = ch;
    }
    return (strcmp(input_buf, "exit"));
}


char*** parse_command(char* input, int* count){
    char*** commands = 0;
    int ph_count = 0;
    int wr_count = 0;
    int ch_count = 0;
    int word     = 0;
    int phrase   = 0;

    for(int i = 0; input[i] != 0; i++){
        if (!word && input[i] == ' ')
            continue;
        if (!word && !phrase){//comand level
            wr_count = 0;
            ph_count++;
            phrase++;
            commands = (char***)realloc(commands, sizeof(char**) *(ph_count));
            commands[ph_count-1] = 0;
        }
        if (!word){//word level
            word++;
            wr_count++;
            ch_count = 0;
            commands[ph_count-1] = (char**)realloc(commands[ph_count-1], sizeof(char*)*(wr_count+1));
            commands[ph_count-1][wr_count-1] = (char*)calloc(WORD, sizeof(char));
            commands[ph_count-1][wr_count] = 0;
        }
        if (input[i] == ' '){
            word--;
            continue;
        }
        if (input[i] == '|'){
            word--;
            phrase--;
            commands[ph_count-1][wr_count-1][ch_count++] = 0;
            commands[ph_count-1][wr_count] = 0;
            continue;
        }
        commands[ph_count-1][wr_count-1][ch_count++] = input[i];
    }
    *count = ph_count;

    return commands;
}


void free_comand(char*** command, int len){
    for(int i = 0; i<len;i++){
        for(int j = 0; command[i][j] != 0;j++){
            free(command[i][j]);
        }
        free(command[i]);
    }
    free(command);

}


int run_command(char** proc){
    int pipefd[2] = {};
    pipe(pipefd);

    int pid = fork();
    if (pid == 0){
        dup2(pipefd[1], 1);
        execvp(proc[0], proc);
        exit(0);
    }else{
        close(pipefd[1]);
    }

    return 0;
}


void print_hello(){
    printf("Dmitry's Shell (C) MIPT Corporation. All rights reserved.\n\n");

    printf("Try the new cross-platform Shell (https://github.com/DmSM-1/shell)\n");
}


void dump(char* input, char*** commands, int count, int err_flag){
    printf("Shell\n");
    printf("===================\n");
    printf("Input:      %s\n", input);
    printf("Count:      %d\n", count);
    printf("Commands:\n");
    for(int j = 0; j < count; j++){
        printf("[%d]: ", j);
        int count = 0;
        for(int i = 0; commands[j][i] != 0; i++){
            printf("'%s', ", commands[j][i]);
            count = i;
        }
        printf("(len: %d)\n", count+1);
    }
    printf("Err_flag:   %d\n", err_flag);
    printf("===================\n");

}



int main(){
    char    input_buf[COPACITY] = {0};
    char*** command_buf;
    int     command_count   = 0;
    int     err_flag        = 0;
    
    int getg[16];
    int len = 16;
    int uid = (int)getuid();
    struct passwd* pw = getpwuid(uid);

    print_hello();
    printf("%s >>> ", pw->pw_name);
    while(read_to(input_buf)){
        command_buf = parse_command(input_buf, &command_count);

        int pipefd[2] = {};
        int old_pipefd[2] = {};
        
        for (int i = 0; i < command_count && command_buf[i][0][0] != 0; i++){
            pipe(pipefd);
            int pid = fork();
            if (pid == 0){
                if(i>0){
                    dup2(old_pipefd[0], 0);
                    close(old_pipefd[0]);
                }
                if (i<command_count-1)
                    dup2(pipefd[1], 1);
                execvp(command_buf[i][0], command_buf[i]);
                return -1;
            }else{
                 close(pipefd[1]);
                 wait(&pid);
                 old_pipefd[0] = pipefd[0];
            }   
        }
        printf("%s >>> ", pw->pw_name);
        free_comand(command_buf, command_count);
    }   
    return 0;
}
