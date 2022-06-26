#include "hist.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/utsname.h>
#include "hist.c"

#define MAXLINE 1024

void cmd_fin(char**);
void cmd_carpeta(char**);
void cmd_autores(char**);
void cmd_pid(char**);
void cmd_fecha(char**);
void cmd_infosis(char**);
void cmd_ayuda(char**);
void cmd_comando(char**);
void cmd_hist(char**);

tHist list;

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
        {"fecha", cmd_fecha},
        {"infosis", cmd_infosis},
        {"ayuda", cmd_ayuda},
        {"comando", cmd_comando},
        {"hist", cmd_hist},
        {NULL, NULL}
};


void cmd_autores(char **tokens){

    if (tokens[1] == NULL)
        printf("Alejandro Fernandez Vazquez    a.fernandez9@udc.es\nYago Fernandez Rego            yago.fernandez.rego@udc.es\n");
    else if (!strcmp(tokens[1], "-l"))
        printf("a.fernandez9@udc.es\nyago.fernandez.rego@udc.es\n");
    else if (!strcmp(tokens[1], "-n"))
        printf("Alejandro Fernandez Vazquez\nYago Fernandez Rego\n");
    else printf("Command %s %s not found\n", tokens[0], tokens[1]);

}


void cmd_carpeta(char **tokens){

    char dir[MAXLINE];

    if (tokens[1] == NULL)
        printf("%s\n", getcwd(dir, MAXLINE));
    else {
        if (chdir(tokens[1]) == -1)
            perror("Cannot change directory: Permission denied\n");
        else
            printf("%s\n", getcwd(dir, MAXLINE));
    }
}


void cmd_pid(char **tokens){

    if (tokens[1] == NULL)
        printf("Shells process pid: %d\n", getpid());
    else if (!strcmp(tokens[1], "-p"))
        printf("Shell parent process pid: %d\n", getppid());
    else
        printf("Command %s %s not found\n", tokens[0], tokens[1]);
}

void cmd_fecha(char **tokens){

    time_t T = time(NULL);
    struct tm tm = *localtime(&T);

    if (tokens[1] == NULL) {
        printf("System Date is: %02d/%02d/%04d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
        printf("System Time is: %02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
    else if (!strcmp(tokens[1], "-d"))
        printf("System Date is: %02d/%02d/%04d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
    else if (!strcmp(tokens[1], "-h"))
        printf("System Time is: %02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
    else
        printf("Command %s %s not found\n", tokens[0], tokens[1]);
}

void cmd_infosis(char **tokens){
    struct utsname unameData;
    uname(&unameData);
    printf("Name of the system: %s\nName of this node: %s\nCurrent release: %s\nCurrent version: %s\nHardware type: %s\n",
           unameData.sysname, unameData.nodename, unameData.release, unameData.version, unameData.machine);
}

void cmd_ayuda(char **tokens) {
    if (tokens[1] == NULL) {
        printf("All the avaliable commands are: \n");
        for (int i = 0; C[i].name != NULL; i++) {
            printf("%s\n", C[i].name);
        }
    } else {
        if ((0 == (strcmp(tokens[1], "fin"))) || (0 == (strcmp(tokens[1], "salir"))) ||
            (0 == (strcmp(tokens[1], "bye"))))
            printf("This command ends the shell\n");

        else if ((0 == (strcmp(tokens[1], "autores")))) {
            printf("The command %s gives info on the authors of the code, ", tokens[1]);

            if (tokens[2] == NULL) {
                printf("by default both the names and the logins will be printed\n");
                printf("there exist two variations of this command, <<autores -p>> and <<autores -n>>\n");
            } else if ((0 == (strcmp(tokens[2], "-l")))) {
                printf("the parameter %s specifies that only the logins will be printed\n", tokens[2]);
            } else if ((0 == (strcmp(tokens[2], "-n")))) {
                printf("moreover, the parameter %s specifies that only the names will be printed\n", tokens[2]);
            } else {
                printf("the parameter %s, however, does not exist for this command\n", tokens[2]);
            }
        } else if ((0 == (strcmp(tokens[1], "fecha")))) {
            printf("The command %s gives info on the current time,", tokens[1]);

            if (tokens[2] == NULL) {
                printf("by default the date and the time are shown ");
                printf("there exist two variations of this command, <<fecha -d>> and <<fecha -h>>\n");
            } else if ((0 == (strcmp(tokens[2], "-d")))) {
                printf(" the parameter %s specifies that only the date will be printed\n", tokens[2]);
            } else if ((0 == (strcmp(tokens[2], "-h")))) {
                printf("moreover, the parameter %s specifies that only the hour will be printed\n", tokens[2]);
            } else {
                printf("the parameter %s, however, does not exist for this command\n", tokens[2]);
            }
        } else if ((0 == (strcmp(tokens[1], "carpeta")))) {
            printf("The command %s gives info about folders,\n", tokens[1]);

            if (tokens[2] == NULL) {
                printf("by default the date and the time are shown\n ");
                printf("there exist a extra variation of this command, <<carpeta [directory]>> \n");
            } else {
                printf("when used with an extra parameter, this command changes the current folder to the given parameter %s\n",
                       tokens[2]);
            }
        }
    }
}

void cmd_hist(char **tokens) {

    int count = 1;
    int N;
    if (tokens[1] != NULL_COMMAND) N = (int) strtol(tokens[1], tokens, 0);
    tCommand_pos prev;

    if (tokens[1] == NULL_COMMAND) {
        for (tCommand_pos pos = first(list); pos != NULL_COMMAND; pos = next(pos, list)) {

            printf("%d->%s\n", count, getItem(pos, list).command);
            count ++;
        }
    }
    else if (0==(strcmp(tokens[1], "-c"))) {
        if(list == NULL){
            printf("There are no commands stored in memory\n");
            return;
        }
        for (tCommand_pos i = first(list); i != NULL_COMMAND;) {
            prev = i;
            i = next(i, list);
            deleteAtPosition(prev, &list);
        }
    }
    else if (N < 0) {
        for (tCommand_pos pos = first(list); ((pos != NULL_COMMAND) && (count <= abs(N))); pos = next(pos, list)) {
            printf("%d->%s\n", count, getItem(pos, list).command);
            count ++;
        }
    }
    else
        printf("Command %s %s not found\n", tokens[0], tokens[1]);

};

void cmd_fin(char **tokens) {
    exit(1);
}

void processInput(char **tokens, char str[]) {

    int i;
    struct tNode node;
    char command[COMMAND_LENGTH_LIMIT];

    if (tokens[0] == NULL)
        return;

    for (i = 0 ; C[i].name != NULL ; i++){
        if (!strcmp(tokens[0], C[i].name)){
            node.next = NULL_COMMAND;



            char strc[MAXLINE];
            strcpy(strc, str);

            strcpy(command, strc);

            /*
            if (tokens[1] == NULL_COMMAND)
                strcpy(command, tokens[0]);



            else if (tokens[2] == NULL_COMMAND) {
                strcpy(command, tokens[0]);
                strcat(command, " ");
                strcat(command, tokens[1]);
                printf("%s",tokens[1]);
            }
            else{
                strcpy(command, tokens[0]);
                strcat(command, " ");
                strcat(command, tokens[1]);
                strcat(command, " ");
                strcat(command, tokens[2]);
            }
            */
            strcpy(node.data.command, command);
            insertItem(node, &list);

            (C[i].func)(tokens);
//            tokens[1] = NULL_COMMAND;
            break;
        }
    }
    if (C[i].name == NULL)
        printf("Command %s not found\n", tokens[0]);
}



//this function is used to split the string (named previously as trocearCadena())
//and to count the number of words written
char splitString(char str[], char **tokens){


    char strc[MAXLINE];
    strcpy(strc, str);
    int num_words = 0;
    char delim[] = " \t\a\r\n";

    char *ptr = strtok(strc, delim);

    int i=0;

    while(ptr != NULL){
        num_words++;
        tokens[i]=ptr;
        i++;
        ptr = strtok(NULL, delim);
    }
    return **tokens;
}

void cmd_comando(char **tokens) {

    int N;
    int count = 0;
    tCommand_pos i;

    if (tokens[1] != NULL_COMMAND) {
        N = (int) strtol(tokens[1], tokens, 0);
        if (N > 0){

            for (i = first(list); ((i != NULL_COMMAND) && (count != N)); i = next(i, list))
                count ++;

            if (i != NULL_COMMAND) {
                tokens[1] = NULL_COMMAND;
                printf("%s\n", getItem(previous(i, list), list).command);
                splitString(getItem(previous(i, list), list).command, tokens);
                processInput(tokens, "");
            }
            else
                printf("");

        } else
            printf("");
    } else
        printf("\n");
}


int main(){
    char str[MAXLINE];
    char **tokens = malloc(MAXLINE);

    createEmptyList(&list);

    while(1) {
        printf("*) ");
        fgets(str, MAXLINE, stdin);
        str[strcspn(str, "\n")] = 0;
        splitString(str, tokens);
        processInput(tokens,str);
    }

    return 0;
}