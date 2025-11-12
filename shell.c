#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUM_ARGS 100
#define BUFFER_SIZE 1000
int main(){
    char *args[NUM_ARGS];
    char buffer[BUFFER_SIZE];
    while(1){
        printf("JohnShell> ");
        if(fgets(buffer, sizeof(buffer), stdin) == NULL)
            break;
        
        buffer[strcspn(buffer, "\n")] = '\0';

        if(strcmp(buffer, "exit"))
            break;

        int i = 0;
        char *token = strtok(buffer," ");
        while(token != NULL && i < NUM_ARGS){
            args[i++] = token;
            token = strtok("NULL", " ");
        }
        pid_t pid = fork();
        if(pid == 0){
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