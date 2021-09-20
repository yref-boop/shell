#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

#define MAXLINE 1024
/*
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

void cmd_autores(char *tr[]){           //modificado *tr a *tr[]
    printf("los autores son... \n");
};

void cmd_carpeta(char *tr[]){
    char dir[MAXLINE];

    if(tr[1] == NULL)
        printf("%s\n", getcwd(dir, MAXLINE));
    else
        if(chdir(tr) == -1)
            perror("Cannot change directory: Permission denied\n");
}

void cmd_pid(char *tr[]){
    if(tr[0] == NULL)
        printf("Shell parent process pid: %d\n", getpid());
    else
        printf("Shells process pid: %d\n", getpid());
}

void cmd_fin(){
    exit(0);
}

void processInput(char *split[]){

    int i;

    if(split[0]==NULL)
        return;

    for(i = 0 ; C[i].name != NULL ; i++){
        if(!strcmp(split[0], C[i].name))
           (C[i], func)(split+i);
        return;
    }

    printf("Command not found \n", split[0]);
}

*/
//this function is used to split the string (named previously as trocearCadena())
//and to count the number of words written
char splitString(char str[], char *token[]){
    char strc[MAXLINE];
    strcpy(strc,str);
    int num_words = 0;
    char delim[] = " \t\a\r\n";

    char *ptr = strtok(strc, delim);

    char **tokens = malloc(MAXLINE);
    int i=0;

    while(ptr != NULL){
        num_words++;
        int size_ptr = sizeof(ptr);

        tokens[i]=ptr;
        i++;

        char x [size_ptr];
        strcpy(x ,ptr);
        printf("'%s'\n", ptr);
        ptr = strtok(NULL, delim);

    }
    printf("%d\n", num_words);
    printf("'%s'\n", tokens[1]);

    return tokens;
}


int main(){
    char str[MAXLINE];
    char token[MAXLINE/2];

    while(1){
        printf("$ ");
        fgets(str, MAXLINE, stdin);
        str[strcspn(str, "\n")] = 0;

        splitString(str, token);
       // processInput(str);
    }

    return 0;
}