#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

#define MAXLINE 1024

void cmd_fin(char**);
void cmd_carpeta(char**);
void cmd_autores(char**);
void cmd_pid(char**);


struct CMD{
    char *name;
    void(*func)(char**);
};

struct CMD C[]={
        {"fin", cmd_fin},
        {"bye", cmd_fin},
        {"salir", cmd_fin},
        {"carpeta", cmd_carpeta},
        {"autores", cmd_autores},
        {"pid", cmd_pid},
        {NULL, NULL}
};


void cmd_autores(char *tokens[]){
    printf("los autores son ...  \n");
}


void cmd_carpeta(char **tokens){
    char dir[MAXLINE];

    if(&tokens[1] == NULL)
        printf("%s\n", getcwd(dir, MAXLINE));
    else {
        if (chdir(*tokens) == -1)
            perror("Cannot change directory: Permission denied\n");
        else
            printf("%s\n", getcwd(dir, MAXLINE));
    }
}


void cmd_pid(char *tokens[]){
    if(tokens[0] == NULL)
        printf("Shell parent process pid: %d\n", getpid());
    else
        printf("Shells process pid: %d\n", getpid());
}


void cmd_fin(char *tokens[]) {
    exit(1);
}


void processInput(char **tokens){
    int i;

    if (tokens[0] == NULL )
        return;

    for(i = 0 ; C[i].name != NULL ; i++){
        if(!strcmp(tokens[0], C[i].name )){
            (C[i].func)(tokens);
            break;
        }
    }
    if(C[i].name == NULL )
        printf("Command %s not found\n", tokens[0]);
}




//this function is used to split the string (named previously as trocearCadena())
//and to count the number of words written
char splitString(char str[], char **tokens){
    char strc[MAXLINE];
    strcpy(strc,str);
    int num_words = 0;
    char delim[] = " \t\a\r\n";

    char *ptr = strtok(strc, delim);

    int i=0;

    while(ptr != NULL){
        num_words++;
        tokens[i]=ptr;
        i++;
        //printf("'%s'\n", ptr);
        ptr = strtok(NULL, delim);
    }
    //printf("%d\n", num_words);
    //printf("'%s'\n", tokens[0]);

    return **tokens;

}



int main(){
    char str[MAXLINE];
    char **tokens = malloc(MAXLINE);

    while(1){
        printf("$ ");
        fgets(str, MAXLINE, stdin);
        str[strcspn(str, "\n")] = 0;
        splitString(str, tokens);
        processInput(tokens);

    }

    return 0;
}