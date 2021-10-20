
#include "hist.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>

#define MAXLINE 1024

//headers of the functions of each command

void cmd_comando(char**);
void cmd_hist(char**);
void cmd_ayuda(char**);
void cmd_crear(char**);
void cmd_listfich(char**);
void cmd_listdir(char**);
void cmd_carpeta(char**);
void cmd_autores(char**);
void cmd_pid(char**);
void cmd_fecha(char**);
void cmd_infosis(char**);
void cmd_borrar(char**);
int cmd_borrarrec(char**);
void cmd_fin(char**);
void read_directory();

//defines the value of a list in which we will keep the historic commands
tHist list;

//the following struct, C stores all the possible commands for ease of access

struct CMD {
    char *name;
    void(*func)(char**);
};

struct CMD C[]={
        {"comando",   cmd_comando},
        {"hist",      cmd_hist},
        {"ayuda",     cmd_ayuda},
        {"crear",     cmd_crear},
        {"listfich",  cmd_listfich},
        {"listdir",   cmd_listdir},
        {"carpeta",   cmd_carpeta},
        {"autores",   cmd_autores},
        {"pid",       cmd_pid},
        {"fecha",     cmd_fecha},
        {"infosis",   cmd_infosis},
        {"borrar",    cmd_borrar},
        {"borrarrec", (void (*)(char **)) cmd_borrarrec},
        {"fin",       cmd_fin},
        {"bye",       cmd_fin},
        {"salir",     cmd_fin},
        {NULL, NULL}
};



//function corresponding to the command pid
//pid gives info on the current process, pid -p on ints parent process

void cmd_pid(char **tokens) {

    if (tokens[1] == NULL)
        printf("Shells process pid: %d\n", getpid());
    else if (!strcmp(tokens[1], "-p"))
        printf("Shell parent process pid: %d\n", getppid());
    else
        printf("Command %s %s not found\n", tokens[0], tokens[1]);
}


//function corresponding to the command fecha
//without arguments it prints date and time, fecha -d date and fecha -h time

void cmd_fecha(char **tokens) {

    time_t T = time(NULL);
    struct tm tm = *localtime(&T);

    if (tokens[1] == NULL) {
        printf("System Date is: %02d/%02d/%04d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
        printf("System Time is: %02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
    //if there is a second parameter
    else if (!strcmp(tokens[1], "-d"))
        printf("System Date is: %02d/%02d/%04d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
    else if (!strcmp(tokens[1], "-h"))
        printf("System Time is: %02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
    else
        printf("Command %s %s not found\n", tokens[0], tokens[1]);
}


//function corresponding to the command infosis
//it gives info on the current machine

void cmd_infosis(char **tokens) {
    struct utsname unameData;
    uname(&unameData);
    printf("Name of the system: %s\nName of this node: %s\nCurrent release: %s\nCurrent version: %s\nHardware type: %s\n",
           unameData.sysname, unameData.nodename, unameData.release, unameData.version, unameData.machine);
}


//function corresponding to the command ayuda:
//ayuda displays a list of available commands
//ayuda cmd gives a brief help on the usage of the command cmd

void cmd_ayuda(char **tokens) {

    //if the command is ayuda: print all avaliable commands (each item on the command struct)
    if (tokens[1] == NULL){
        printf("All the avaliable commands are: \n");

        for (int i = 0 ; C[i].name != NULL ; i++){
            printf("%s ", C[i].name);
        }
        printf("\n");
    }

    //if the command contains another command (ayuda cmd) we need to study each case and its parameters:
    else {
        //salir, fin and bye cannot have more parameters
        if( (0==(strcmp(tokens[1], "fin"))) || (0==(strcmp(tokens[1], "salir"))) || (0==(strcmp(tokens[1], "bye")))){
            if(tokens[2]!=NULL){
                printf("Too many arguments\n");
                return;
            }
            printf("This command ends the shell\n");
        }

        //the command autores can have -l and -n as parameters
        else if((0==(strcmp(tokens[1], "autores")))){
            printf("The command %s gives info on the authors of the code, ", tokens[1]);

            //it can never have 3 parameters
            if(tokens[3]!=NULL){
                printf("Too many arguments\n");
                return;
            }
            if(tokens[2] == NULL){
                printf("by default both the names and the logins will be printed\n");
                printf("there exist two variations of this command, <<autores -p>> and <<autores -n>>\n");
            }
                //if there exists a third parameter, give info if its is -l or -n else say that the parameter is not correct
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


        //the command fecha can be fecha, fecha -d or fecha -h
        else if ((0==(strcmp(tokens[1], "fecha")))){
            printf("The command %s gives info on the current time,", tokens[1]);

            if(tokens[3]!=NULL){
                printf("Too many arguments\n");
                return;
            }

            //if there exists a third parameter, give info and if its is -d or -h give specifics
            if(tokens[2] == NULL){
                printf("by default the date and the time are shown ");
                printf("there exist two variations of this command, <<fecha -d>> and <<fecha -h>>\n");
            }

            //if there exists a third parameter, give info if its is -d or -h else say that the parameter is not correct
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

        //the command carpeta can be carpeta or carpeta<directory>
        else if((0==(strcmp(tokens[1], "carpeta")))){
            printf("The command %s gives info about folders,\n", tokens[1]);

            if(tokens[3]!=NULL){
                printf("Too many arguments\n");
                return;
            }

            if(tokens[2] == NULL){
                printf("by default the date and the time are shown\n ");
                printf("there exist a extra variation of this command, <<carpeta [directory]>> \n");
            }

            //if there exists a third parameter give info
            else{
                printf("when used with an extra parameter, this command changes the current folder to the given parameter %s\n", tokens[2]);
            }
        }

        //pid can either be pid or pid -p
        else if((0==(strcmp(tokens[1], "pid")))){
            printf("The command %s gives info about the current process ID,\n", tokens[1]);

            if(tokens[3] != NULL_COMMAND){
                printf("Too many arguments\n");
                return;
            }

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

        //the command hist can either be his, hist -c or hist -N
        else if((0==(strcmp(tokens[1], "hist")))) {
            printf("The command %s gives info about the commands previosly executed on the shell,\n", tokens[1]);

            if(tokens[3]!=NULL){
                printf("Too many arguments\n");
                return;
            }


            if (tokens[2] == NULL) {
                printf("by default it prints all the commands that have been input and their order number\n ");
                printf("there exist two extra variations of this command, <<hist -c>>> and <<hist -N>> \n");
            }

                //if there exists a third parameter, give info if its is -c or -N else say that the parameter is not correct
            else {
                int N;
                N = (int) strtol(tokens[1], tokens, 0);
                if ((0 == (strcmp(tokens[2], "-c")))) {
                    printf("when used with %s parameter, the command clears the list of historic commands\n",
                           tokens[2]);
                } else if ((0 == (N<0))) {
                    printf("when used with an number N as parameter, the command prints history of commands previous to Nth\n");
                } else {
                    printf("the parameter %s, however, does not exist for this command\n", tokens[2]);
                }
            }
        }

        //the command infosis does not have variations
        else if((0==(strcmp(tokens[1], "infosis")))) {
            printf("the command %s gives info about the current system, there are no variations of this command\n",
                   tokens[1]);

            if (tokens[2] != NULL) {
                printf("Too many arguments\n");
                return;
            }
        }

        //the command comando does not have variations
        else if((0==(strcmp(tokens[1], "comando")))) {
            printf("this command executes the command on the list with the wanted position\n");

            if (tokens[2] != NULL) {
                printf("Too many arguments\n");
                return;
            }
        }

        //the command ayuda can either be ayuda or ayuda <command>
        else if((0==(strcmp(tokens[1], "ayuda")))){
            printf("The command %s gives a brief summary of the commands,\n", tokens[1]);

            if(tokens[2] == NULL){
                printf("by default it lists all the possible commands\n ");
                printf("there exist an extra variations of this command, <<ayuda -cmd>>\n");
            }

            //if there exists a third parameter, give info on it
            else if (tokens[2]!=NULL) {
                printf("when used with an extra parameter, the command gives info about an specific command, if it exists \n");
            }
        }

        //the command crear can be either crear <name> or crear -f <name>
        else if((0==(strcmp(tokens[1], "crear")))){
            printf("The command %s <name> creates a file named <name>,\n", tokens[1]);

            if(tokens[2] == NULL){
                printf("by default it creates a file\n ");
                printf("there exist an extra variations of this command: crear -f <name>\n");
            }

            //if there exists a third parameter, give info on it
            else if ((0 == (strcmp(tokens[2], "-f")))) {
                printf("when used with %s parameter, the command creates a folder\n",
                       tokens[2]);
            }
        }

        //the command borrarrec <name> <name>... does not have variations
        else if((0==(strcmp(tokens[1], "borrarrec")))){
            printf("this command deletes files and/or non empty directories with all their contents\n");
            printf("if given without names, prints the current working directory\n");
        }

        //the command borrar <name> <name>... does not have variations
        else if((0==(strcmp(tokens[1], "borrar")))) {
            printf("this command deletes files and/or empty directories\n");
            printf("if given without names, prints the current working directory\n");
        }

        //the command listfich can be listfich, listfich -long <names> , listfich -link <names> or  listfich -acc <names>
        else if ((0==(strcmp(tokens[1], "listfich")))){
            printf("The command %s gives info on files, directories or devices specified,", tokens[1]);
            printf("if given without names, prints the current working directory\n");

                //if there exists a third parameter, give info if its is -d or -h else say that the parameter is not correct
            if ((0==(strcmp(tokens[2], "-long")))){
                printf("%s implies long listing following the format:\n", tokens[2]);
                printf("date number of links (inode number) owner group mode size name\n");
            }
            else if ((0==(strcmp(tokens[2], "-link")))){
                printf("with %s , if the file is a symbolic link it prints the directory to which it points\n", tokens[2]);
            }
            else if ((0==(strcmp(tokens[2], "-acc")))){
                printf("%s prints last access date instead of last modification date\n", tokens[2]);
            }
        }

        //the command listdir can be listidir, listdir -long <names> , listdir -link <names> , listdir -acc <names>
        //listdir -reca <names> , listdir -recb <names> , listdir -hid <names>
        else if ((0==(strcmp(tokens[1], "listdir")))){
            printf("The command %s gives info on files, directories or devices specified,", tokens[1]);
            printf("if name is a directory, it will print its contents\n");
            printf("if given without names, prints the current working directory\n");

            //if there exists a third parameter, give info if its is -d or -h else say that the parameter is not correct
            if ((0==(strcmp(tokens[2], "-long")))){
                printf("%s implies long listing following the format:\n", tokens[2]);
                printf("date number of links (inode number) owner group mode size name\n");
            }
            else if ((0==(strcmp(tokens[2], "-link")))){
                printf("with %s , if the file is a symbolic link it prints the directory to which it points\n", tokens[2]);
            }
            else if ((0==(strcmp(tokens[2], "-acc")))){
                printf("%s prints last access date instead of last modification date\n", tokens[2]);
            }
            else if ((0==(strcmp(tokens[2], "-reca")))){
                printf("%s implies that the contents wil be listed recursively after:\n", tokens[2]);
                printf("all the files in de  directory\n");
            }
            else if ((0==(strcmp(tokens[2], "-recb")))){
                printf("as with -reca, %s , prits the subcontents before the parent directory\n", tokens[2]);
            }
            else if ((0==(strcmp(tokens[2], "-hib")))){
                printf("%s also prints hidden files\n", tokens[2]);
            }
        }




        //if there is a second parameter but does not coincide with any:
        else if (tokens[1]!=NULL){
            printf("The command %s cannot be found\n", tokens[1]);
        }
    }
}


//function corresponding to the command hist:
//shows(hist/(hist N)) / clears(hist -c) the historic commands executed

void cmd_hist(char **tokens) {

    int count = 1;
    int N;

    //if exists, store the int value of the second  token inside a variable N
    if (tokens[1] != NULL_COMMAND) N = (int) strtol(tokens[1], tokens, 0);

    //if the command is only hist: print all the list from the beginning
    if (tokens[1] == NULL_COMMAND) {
        for (tCommand_pos pos = first(list); pos != NULL_COMMAND; pos = next(pos, list)) {
            printf("%d->%s\n", count, getItem(pos, list).command);
            count ++;
        }
    }
    //if the command is hist -c, delete the list
    else if (0==(strcmp(tokens[1], "-c"))) {
        //if already empty, do nothing
        if(list == NULL){
            printf("There are no commands stored in memory\n");
            return;
        }
            //position needed on next for
            tCommand_pos prev;

        //delete one by one until empty, starting at first position
        for (tCommand_pos i = first(list); i != NULL_COMMAND;) {
            prev = i;
            i = next(i, list);
            deleteAtPosition(prev, &list);
        }
    }
    //if the command is a number, print from first until the Nth or the list reaches its end
    else if (N < 0) {
        for (tCommand_pos pos = first(list); ((pos != NULL_COMMAND) && (count <= abs(N))); pos = next(pos, list)) {
            printf("%d->%s\n", count, getItem(pos, list).command);
            count ++;
        }
    }
    else
        printf("Command %s %s not found\n", tokens[0], tokens[1]);
}


//this function is the fin command: it ends the shell

void cmd_fin(char **tokens) {
    exit(1);
}


//this function that has the already tokenized string as input
//and decides what command functions to execute

void processInput(char **tokens, char str[]) {

    int i;
    struct tNode node;

    //if the input is empty (the \n was previously erased) then do nothing
    if (tokens[0] == NULL_COMMAND) return;

    //for each position on the struct C that contains the commands,
    for (i = 0 ; C[i].name != NULL ; i++){

        //if there is a match between the token and one of the commands
        if (!strcmp(tokens[0], C[i].name)) {

            //create a node and store the whole string on the list that stores the hist of the program
            node.next = NULL_COMMAND;
            //strcpy(node.data.command, str);

            //if at least one item has correct data, store on list
            if (i > 1)
                insertItem(node, &list);

                //since it is impossible that some commands have 3 or more arguments...
            if (((i < 2) || (i > 5)) && (i != 9) && (tokens[2] != NULL_COMMAND))
                printf("Too many arguments\n");
            else
                (C[i].func)(tokens); //the struct C is consulted to get the current function

                //since the function is already finished, the tokens are not needed, thus their content is erased
            for (int j = 0; tokens[j] != NULL; ++j)
                tokens[j] = NULL;
            break;
        }
    }
    //if the struct C has been completely checked without any being the same as the input, then the input is not an implemented command
    if (C[i].name == NULL)
        printf("Command %s not found\n", tokens[0]);
}


//this function is used to split the string (named previously as trocearCadena())
//and to count the number of words written

char splitString(char str[], char **tokens){

    //make a copy of str named strc to avoid possible mutations on the string
    char strc[MAXLINE];
    strcpy(strc, str);

    //create delimiters that separate each word
    char delim[] = " \t\a\r\n";

    //using the function strtok to tokenize strc
    char *ptr = strtok(strc, delim);

    //until the string ends, divide string by its delimiters and store ordely into tokens[]
    int i=0;
    while(ptr != NULL){
        tokens[i] = ptr;
        i++;
        ptr = strtok(NULL, delim);
    }

    //once finished, all tokens are returned
    return **tokens;
}


//this function is for the command comando:
//repeats command number N from history list

void cmd_comando(char **tokens) {

    int N;
    //starting at the first position of the historic list
    tCommand_pos i = first(list);
    //if it is not null (it exists)
    if (tokens[1] != NULL_COMMAND) {

        //strtoll converts the second token (containing the number) to an integer number and stores it in the variable N
        N = (int) strtol(tokens[1], tokens, 0);

        if (N > 0) {

            //look for the command N in the list (break if NULL command reached)
            for (int j = 1; j < N; ++j) {
                if (i == NULL_COMMAND) break;
                i = next(i, list);
            }

            //if found, get the info from the corresponding node, and execute splitString and processInput with retrieved data
            if ((tokens[0] != NULL_COMMAND) && (i != NULL_COMMAND)) {
                tokens[1] = NULL_COMMAND;
                printf("%s\n", getItem(i, list).command);
                splitString(getItem(i, list).command, tokens);
                processInput(tokens, getItem(i, list).command);
            }
            else
                printf("Command not found\n");
        } else
            printf("Command not found\n");
    } else
        printf("No command numbeer has been inserted");
}


int main(){
    char str[MAXLINE]; //variable which stores the input
    char **tokens [MAXLINE]; //pointer to pointer of chars: array of arrays that will contain each separated command
    createEmptyList(&list);  //list needed for command hist

    while(1) {
        printf("*) ");
        fgets(str, MAXLINE, stdin); //store input in str
        if (strcmp(str,"\n")) {     //until new line
            str[strcspn(str, "\n")] = 0;//eliminate the new line char from string to ease its management
            splitString(str, tokens);   //divides the string into each part and stores on tokens
            processInput(tokens, str);  //processes the tokens
        }
        memset(str, '0', MAXLINE);   //erase all contents on str for next iteration
    }
    return 0;
}