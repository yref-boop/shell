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

int trocearCadena(char *cadena, char *trozos) {
    int i = 1;
            if((trozos[0] = strtok(cadena, "\n\t")) == NULL)
                return 0;
            while((trozos[i] = strtok(NULL, "\n\t")) != NULL)
                i++;
            return i;
}


void procesarEntrada(char *tr[]){
    int i;

    if(tr[0] == NULL)
        return;
    for(i=0; C[i].name != NULL; i++){
        if(!strcmp(tr[0], C[i].name))
            (*C[i], func)(tr+i);
        return;
    }

    printf("Command not found \n", tr[0]);
}



int main(){
    char linea[MAXLINE];
    char *tr[MAXLINE/2];

    while(1){
        printf("$ ");
        fgets(linea, MAXLINE, stdin);
        trocearCadena(linea, tr);

        procesarEntrada(char *tr[]);
    }

    return 0;
}
