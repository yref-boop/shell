#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/utsname.h>


#define MAXLINE 1024

void cmd_fin(char**);
void cmd_carpeta(char**);
void cmd_autores(char**);
void cmd_pid(char**);
void cmd_fecha(char**);
void cmd_ayuda(char**);
void cmd_infosis(char**);


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
        {"ayuda", cmd_ayuda},
        {"infosis", cmd_infosis},
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
    else if (0 == (strcmp(tokens[1], "-p")))
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
    else if (0 == (strcmp(tokens[1], "-d")))
        printf("System Date is: %02d/%02d/%04d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
    else if (0 == (strcmp(tokens[1], "-h")))
        printf("System Time is: %02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
    else
        printf("Command %s %s not found\n", tokens[0], tokens[1]);
}

void cmd_fin(char **tokens) {
    exit(1);
}

void cmd_infosis(char **tokens){
    struct utsname unameData;
    uname(&unameData);
    printf("Name of the system: %s\nName of this node: %s\nCurrent release: %s\nCurrent version: %s\nHardware type: %s\n", unameData.sysname, unameData.nodename, unameData.release, unameData.version, unameData.machine);
}

void cmd_ayuda(char **tokens){
    if (tokens[1] == NULL){
        printf("All the avaliable commands are: \n");
        for (int i = 0 ; C[i].name != NULL ; i++){
            printf("%s\n", C[i].name);
        }
    }
    else {
        if( (0==(strcmp(tokens[1], "fin"))) || (0==(strcmp(tokens[1], "salir"))) || (0==(strcmp(tokens[1], "bye"))))
            printf("This command ends the shell");

        else if((0==(strcmp(tokens[1], "autores")))){
            printf("The command %s gives info on the authors of the code, ", tokens[1]);

            if(tokens[2] == NULL){
                printf("by default both the names and the logins will be printed\n");
                printf("there exist two variations of this command, <<autores -p>> and <<autores -n>>\n");
            }

            else if ((0==(strcmp(tokens[2], "-l")))){
                printf("the parameter %s specifies that only the logins will be printed\n", tokens[2]);
            }

            else if ((0==(strcmp(tokens[2], "-n")))){
                printf("moreover, the parameter %s specifies that only the names will be printed\n", tokens[2]);
            }
            else{
                printf("the parameter %s, however, does not exist for this command\n", tokens[2]);
            }
        }

        else if ((0==(strcmp(tokens[1], "fecha")))){
            printf("The command %s gives info on the current time,", tokens[1]);

            if(tokens[2] == NULL){
              printf("by default the date and the time are shown ");
              printf("there exist two variations of this command, <<fecha -d>> and <<fecha -h>>\n");
            }

            else if ((0==(strcmp(tokens[2], "-d")))){
                printf(" the parameter %s specifies that only the date will be printed\n", tokens[2]);
            }
            else if ((0==(strcmp(tokens[2], "-h")))){
                printf("moreover, the parameter %s specifies that only the hour will be printed\n", tokens[2]);
            }
            else{
                printf("the parameter %s, however, does not exist for this command\n", tokens[2]);
            }
        }

        else if((0==(strcmp(tokens[1], "carpeta")))){
            printf("The command %s gives info about folders,\n", tokens[1]);

            if(tokens[2] == NULL){
                printf("by default the date and the time are shown\n ");
                printf("there exist a extra variation of this command, <<carpeta [directory]>> \n");
            }

            else{
                printf("when used with an extra parameter, this command changes the current folder to the given parameter %s\n", tokens[2]);
            }
        }

        else if((0==(strcmp(tokens[1], "pid")))){
            printf("The command %s gives info about the current process ID,\n", tokens[1]);

            if(tokens[2] == NULL){
                printf("by default the process executing the shell is printed\n ");
                printf("there exist a extra variation of this command, <pid -p>> \n");
            }

            else if ((0==(strcmp(tokens[2], "-p")))) {
                printf("when used with %s parameter, the program prints the ID os the parent process of the shell\n", tokens[2]);
            }

            else {
                printf("the parameter %s, however, does not exist for this command\n", tokens[2]);
            }
        }

        else if((0==(strcmp(tokens[1], "hist")))){
            printf("The command %s gives info about the commands previosly executed on the shell,\n", tokens[1]);

            if(tokens[2] == NULL){
                printf("by default it prints all the commands that have been input and their order number\n ");
                printf("there exist two extra variations of this command, <<hist -c>>> and <<hist -N>> \n");
            }

            else if ((0==(strcmp(tokens[2], "-c")))) {
                printf("when used with %s parameter, the command clears the list of historic commands\n", tokens[2]);
            }

            //PARAMETRO N (revisar como lo implementa ferni)


            else {
                printf("the parameter %s, however, does not exist for this command\n", tokens[2]);
            }
        }

        else if((0==(strcmp(tokens[1], "infosis"))))
            printf("the command %s gives info about the current system, there are no variations of this command\n", tokens[1]);

        else if((0==(strcmp(tokens[1], "ayuda")))){
            printf("The command %s gives a brief summary of the commands,\n", tokens[1]);

            if(tokens[2] == NULL){
                printf("by default it lists all the possible commands\n ");
                printf("there exist an extra variations of this command, <<ayuda -cmd>>\n");
            }

            else if (tokens[2]!=NULL) {
                printf("when used with an extra parameter, the command gives info about an specific command, if it exists \n");
            }
        }
        else if (tokens[1]!=NULL){
            printf("the command %s cannot be found", tokens[1]);
        }
    }
}


void processInput(char **tokens){
    int i;

    if (tokens[0] == NULL )
        return;

    for (i = 0 ; C[i].name != NULL ; i++){
        if (0 == (strcmp(tokens[0], C[i].name))){
            (C[i].func)(tokens);
            break;
        }
    }
    if (C[i].name == NULL )
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