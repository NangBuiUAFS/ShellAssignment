#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define NUM_ARGS 100
#define BUFFER_SIZE 1000
int main(){
    char *args[NUM_ARGS];
    char buffer[BUFFER_SIZE], input_file[20], output_file[20];
    while(1){
        int hasInput = 0, hasOutput = 0;
        printf("JohnShell> ");
        if(fgets(buffer, sizeof(buffer), stdin) == NULL)
            break;
        
        buffer[strcspn(buffer, "\n")] = '\0';

        if(strcmp(buffer, "exit") == 0)
            break;

        int i = 0;
        char *token = strtok(buffer," ");
        while(token != NULL && i < NUM_ARGS-1){
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
                args[i++] = token;
            }
            token = strtok(NULL, " ");
        }
        args[i] = NULL;
        pid_t pid = fork();
        if(pid == 0){
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
                int fd = open(output_file, O_WRONLY, 0644);
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
        }else{
            wait(NULL);
            printf("Child Finished!\n");
        }
    }
    printf("Exiting Shell\n");
}