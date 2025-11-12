#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define NUM_ARGS 100
#define BUFFER_SIZE 1000
#define NUM_CMDS 10

pid_t child_pids[NUM_CMDS];
int children = 0;


void handle_sigint(int sig){
    for(int i = 0; i < children; i++){
        if(child_pids[i] > 0){
            kill(child_pids[i], SIGINT);
        }
    }
    printf("\nJohnShell> ");
}

int main(){
    char* commands[NUM_CMDS];
    char buffer[BUFFER_SIZE];
    signal(SIGINT, handle_sigint);
    while(1){
        printf("JohnShell> ");
        if(fgets(buffer, sizeof(buffer), stdin) == NULL)
            break;
        
        buffer[strcspn(buffer, "\n")] = '\0';

        if(strcmp(buffer, "exit") == 0)
            break;

        int cmds = 0;
        char *token = strtok(buffer,"|");
        while(token != NULL && cmds < NUM_CMDS){
            commands[cmds++] = token;
            token = strtok(NULL, "|"); 
        }

        int pipes[NUM_CMDS-1][2];
        for(int i = 0; i < cmds-1; i++){
            if(pipe(pipes[i]) < 0){
                perror("pipe failed");
                exit(1);
            }
        }

        for(int i = 0; i < cmds; i++){
            pid_t pid = fork();
            if(pid == 0){
                if(i > 0){
                    dup2(pipes[i-1][0], STDIN_FILENO);
                }
                
                if(i < cmds - 1){
                    dup2(pipes[i][1], STDOUT_FILENO);
                }

                for(int j = 0; j < cmds-1; j++){
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                int k = 0;
                char *args[NUM_ARGS];
                token = strtok(commands[i], " ");
                char input_file[20], output_file[20];
                int hasInput = 0, hasOutput = 0;
                while(token != NULL && k < NUM_ARGS-1){
                    if(strcmp(token,"<") == 0){
                        hasInput = 1;
                        token = strtok(NULL, " ");
                        if(token == NULL){
                            printf("Input file not specified");
                            exit(1);
                        }
                        strcpy(input_file, token);
                    }else if(strcmp(token,">") == 0){
                        hasOutput = 1;
                        token = strtok(NULL, " ");
                        if(token == NULL){
                            printf("Ouput file not specified");
                            exit(1);
                        }
                        strcpy(output_file, token);
                    }else{
                        args[k++] = token;
                    }
                    token = strtok(NULL, " ");
                }
                args[k] = NULL;
                if(hasInput){
                    int fd = open(input_file, O_RDONLY);
                    if(fd < 0){
                        perror("Input file error");
                        break;
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }
                if(hasOutput){
                    int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if(fd < 0){
                        perror("Output file error");
                        break;
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }
                execvp(args[0], args);
                perror("Execvp failed\n");
                exit(1);
            }else {
                child_pids[i] = pid;
                children++;
            }
        }
        for(int i = 0; i < cmds-1; i++){
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        for(int i = 0; i < cmds; i++){
            waitpid(child_pids[i], NULL, 0);
        }
        children = 0;
    }
    printf("Exiting Shell\n");
}