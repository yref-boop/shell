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

void cmd_comando(char**);
void cmd_hist(char**);
void cmd_ayuda(char**);
void cmd_crear(char**);
void cmd_carpeta(char**);
void cmd_autores(char**);
void cmd_pid(char**);
void cmd_fecha(char**);
void cmd_infosis(char**);
void cmd_borrar(char**);
void cmd_borrarrec(char**);
void cmd_listfich(char**);
void cmd_listdir(char**);
void cmd_fin(char**);

tHist list;

struct CMD {
    char *name;
    void(*func)(char**);
};

struct CMD C[]={
        {"comando", cmd_comando},
        {"hist", cmd_hist},
        {"ayuda", cmd_ayuda},
        {"crear", cmd_crear},
        {"carpeta", cmd_carpeta},
        {"autores", cmd_autores},
        {"pid", cmd_pid},
        {"fecha", cmd_fecha},
        {"infosis", cmd_infosis},
        {"borrar", cmd_borrar},
        {"borrarrec", cmd_borrarrec},
        {"listfich", cmd_listfich},
        {"listdir", cmd_listdir},
        {"fin", cmd_fin},
        {"bye", cmd_fin},
        {"salir", cmd_fin},
        {NULL, NULL}
};

void show_dir() {

    char dir[MAXLINE];
    char *a = getcwd(dir, MAXLINE);

    if (a) printf("%s\n", getcwd(dir, MAXLINE));
    else perror("error");
}

void cmd_crear(char **tokens) {

    if (tokens[1] == NULL) {
        show_dir();
        return;
    }

    if (strcmp(tokens[1], "-f") == 0) {
        if(fopen(tokens[2], "w") == NULL)
            printf("Error while trying to create ''\n%m\n");
    } else {
        if(mkdir(tokens[1], 0777) && (errno != EEXIST))
            printf("Error while trying to create ''\n%m\n");
    }

}

void cmd_borrar(char **tokens){

    if (tokens[1] == NULL) {
        show_dir();
        return;
    }

    int token_point = 1;

    while (tokens[token_point] != NULL){
        if(remove(tokens[token_point])==0)
            printf("File deleted successfully \n");
        else printf("File not deleted \n");
        token_point++;
    }
}

void cmd_borrarrec(char **tokens){

    if (tokens[1] == NULL) {
        show_dir();
        return;
    }
}

void show_dir_content(char * path)
{
    DIR * d = opendir(path); // open the path
    if(d==NULL) return; // if was not able, return
    struct dirent * dir; // for the directory entries
    while ((dir = readdir(d)) != NULL) // if we were able to read somehting from the directory
    {
        if(dir-> d_type != DT_DIR) // if the type is not directory just print it with blue color
            printf("%s\n", dir->d_name);
        else
        if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) // if it is a directory
        {
            printf("%s\n", dir->d_name); // print its name in green
            char d_path[255]; // here I am using sprintf which is safer than strcat
            sprintf(d_path, "%s/%s", path, dir->d_name);
            show_dir_content(d_path); // recall with the new path
        }
    }
    closedir(d); // finally close the directory
}

//auxiliar function to get the size of a file
long int aux_file_size(const char* file_name){
    struct stat st;
    if(stat(file_name, &st)==0)
        return (st.st_size);
    else
        return -1;
}


/*
 *  if no options are given: size and name of each file
 *  if no name,  print current working directory
 *  -long: date of last modification(YYYYMMDD-HH:mm), number of links,
 *      owner, group, mode (drwx format), size, name file
 *      if any name is a directory, its info will also be printed
 *      format: date number_of_links (inode_number) owner group mode size name
 *  -link: for long listings: if file is symbolic link the name of the file it point to is also printed
 *      format: date number_of_links (inode_number) owner group mode size name->file_link_points_to
 *  -acc last access time will be used instead of last modification time
 */
void cmd_listfich(char **tokens){

    if (tokens[1] == NULL) {
        show_dir();
        return;
    }



    bool lo, li, ac = false;
    int arg = 1;

    for (; !strcmp(tokens[arg], "-long") || !strcmp(tokens[arg], "-link") || !strcmp(tokens[arg], "-acc"); arg++) {
        switch (tokens[arg][2]) {
            case 'o': lo = true; break;
            case 'i': li = true; break;
            case 'c': ac = true; break;
        }
    }



    for(; tokens[arg]!=NULL; arg++) { //args already has the value of the first file name

        //long int size_file= aux_file_size(tokens[arg]);
        struct stat     st;
        struct passwd   *pwd;
        struct group    *grp;

        if (stat(tokens[arg], &st) != 0)
        {
            printf("Unable to get file properties.\n");
            printf("Please check whether '%s' file exists.\n", tokens[arg]);
            break;
        }

        if (lo == false) {
            //print size and name
            printf("%ld %s\n", st.st_size, tokens[arg]);
        } else {
            if ((li == false) && (ac == false)) {
                //print modification_date number_of_links (inode_number) owner group mode size name
                printf("%s", ctime(&st.st_mtime));
                printf("%ld", st.st_nlink);
                printf("%ld", st.st_ino);

                if ((pwd = getpwuid(st.st_uid)) != NULL)
                    printf(" %-8.8s", pwd->pw_name);
                else
                    printf(" %-8d", st.st_uid);

                if ((grp = getgrgid(st.st_gid)) != NULL)
                    printf(" %-8.8s", grp->gr_name);
                else
                    printf(" %-8d", st.st_gid);

                printf( (S_ISDIR(st.st_mode)) ? "d" : "-");
                printf( (st.st_mode & S_IRUSR) ? "r" : "-");
                printf( (st.st_mode & S_IWUSR) ? "w" : "-");
                printf( (st.st_mode & S_IXUSR) ? "x" : "-");
                printf( (st.st_mode & S_IRGRP) ? "r" : "-");
                printf( (st.st_mode & S_IWGRP) ? "w" : "-");
                printf( (st.st_mode & S_IXGRP) ? "x" : "-");
                printf( (st.st_mode & S_IROTH) ? "r" : "-");
                printf( (st.st_mode & S_IWOTH) ? "w" : "-");
                printf( (st.st_mode & S_IXOTH) ? "x" : "-");

                printf(" %ld %s\n", st.st_size, tokens[arg]);
                break;

            }

            if ((ac == true) && (li == true)) {
                //print access_date number_of_links (inode_number) owner group mode size name->file_links_to

                printf("%s", ctime(&st.st_atime));
                printf("%ld", st.st_nlink);
                printf("%ld", st.st_ino);

                if ((pwd = getpwuid(st.st_uid)) != NULL)
                    printf(" %-8.8s", pwd->pw_name);
                else
                    printf(" %-8d", st.st_uid);

                if ((grp = getgrgid(st.st_gid)) != NULL)
                    printf(" %-8.8s", grp->gr_name);
                else
                    printf(" %-8d", st.st_gid);

                printf( (S_ISDIR(st.st_mode)) ? "d" : "-");
                printf( (st.st_mode & S_IRUSR) ? "r" : "-");
                printf( (st.st_mode & S_IWUSR) ? "w" : "-");
                printf( (st.st_mode & S_IXUSR) ? "x" : "-");
                printf( (st.st_mode & S_IRGRP) ? "r" : "-");
                printf( (st.st_mode & S_IWGRP) ? "w" : "-");
                printf( (st.st_mode & S_IXGRP) ? "x" : "-");
                printf( (st.st_mode & S_IROTH) ? "r" : "-");
                printf( (st.st_mode & S_IWOTH) ? "w" : "-");
                printf( (st.st_mode & S_IXOTH) ? "x" : "-");

                printf("% ld %s -> ", st.st_size, tokens[arg]);
                break;

            }

            if (li == true) {
                //print modification_date number_of_links (inode_number) owner group mode size name->file_links_to
                printf("%s", ctime(&st.st_mtime));
                printf("%ld", st.st_nlink);
                printf("%ld", st.st_ino);

                if ((pwd = getpwuid(st.st_uid)) != NULL)
                    printf(" %-8.8s", pwd->pw_name);
                else
                    printf(" %-8d", st.st_uid);

                if ((grp = getgrgid(st.st_gid)) != NULL)
                    printf(" %-8.8s", grp->gr_name);
                else
                    printf(" %-8d", st.st_gid);

                printf( (S_ISDIR(st.st_mode)) ? "d" : "-");
                printf( (st.st_mode & S_IRUSR) ? "r" : "-");
                printf( (st.st_mode & S_IWUSR) ? "w" : "-");
                printf( (st.st_mode & S_IXUSR) ? "x" : "-");
                printf( (st.st_mode & S_IRGRP) ? "r" : "-");
                printf( (st.st_mode & S_IWGRP) ? "w" : "-");
                printf( (st.st_mode & S_IXGRP) ? "x" : "-");
                printf( (st.st_mode & S_IROTH) ? "r" : "-");
                printf( (st.st_mode & S_IWOTH) ? "w" : "-");
                printf( (st.st_mode & S_IXOTH) ? "x" : "-");

                printf("% ld %s\n", st.st_size, tokens[arg]);
                break;
            }

            if (ac == true) {
                //print access_date number_of_links (inode_number) owner group mode size name
                printf("%s", ctime(&st.st_atime));
                printf("%ld", st.st_nlink);
                printf("%ld", st.st_ino);

                if ((pwd = getpwuid(st.st_uid)) != NULL)
                    printf(" %-8.8s", pwd->pw_name);
                else
                    printf(" %-8d", st.st_uid);

                if ((grp = getgrgid(st.st_gid)) != NULL)
                    printf(" %-8.8s", grp->gr_name);
                else
                    printf(" %-8d", st.st_gid);

                printf( (S_ISDIR(st.st_mode)) ? "d" : "-");
                printf( (st.st_mode & S_IRUSR) ? "r" : "-");
                printf( (st.st_mode & S_IWUSR) ? "w" : "-");
                printf( (st.st_mode & S_IXUSR) ? "x" : "-");
                printf( (st.st_mode & S_IRGRP) ? "r" : "-");
                printf( (st.st_mode & S_IWGRP) ? "w" : "-");
                printf( (st.st_mode & S_IXGRP) ? "x" : "-");
                printf( (st.st_mode & S_IROTH) ? "r" : "-");
                printf( (st.st_mode & S_IWOTH) ? "w" : "-");
                printf( (st.st_mode & S_IXOTH) ? "x" : "-");

                printf("% ld %s\n", st.st_size, tokens[arg]);
                break;
            }
        }
    }
}



void cmd_listdir(char **tokens){

}

void cmd_autores(char **tokens) {

    if (tokens[1] == NULL)
        printf("Alejandro Fernandez Vazquez    a.fernandez9@udc.es\nYago Fernandez Rego            yago.fernandez.rego@udc.es\n");
    else if (!strcmp(tokens[1], "-l"))
        printf("a.fernandez9@udc.es\nyago.fernandez.rego@udc.es\n");
    else if (!strcmp(tokens[1], "-n"))
        printf("Alejandro Fernandez Vazquez\nYago Fernandez Rego\n");
    else printf("Command %s %s not found\n", tokens[0], tokens[1]);

}

void cmd_carpeta(char **tokens) {

    if (tokens[1] == NULL)
        show_dir();
    else {
        if (chdir(tokens[1]) == -1)
            perror("Cannot change directory: Permission denied\n");
        else
            show_dir();
    }
}

void cmd_pid(char **tokens) {

    if (tokens[1] == NULL)
        printf("Shells process pid: %d\n", getpid());
    else if (!strcmp(tokens[1], "-p"))
        printf("Shell parent process pid: %d\n", getppid());
    else
        printf("Command %s %s not found\n", tokens[0], tokens[1]);
}

void cmd_fecha(char **tokens) {

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

void cmd_infosis(char **tokens) {
    struct utsname unameData;
    uname(&unameData);
    printf("Name of the system: %s\nName of this node: %s\nCurrent release: %s\nCurrent version: %s\nHardware type: %s\n",
           unameData.sysname, unameData.nodename, unameData.release, unameData.version, unameData.machine);
}

void cmd_ayuda(char **tokens) {

    if (tokens[1] == NULL){
        printf("All the avaliable commands are: \n");

        for (int i = 0 ; C[i].name != NULL ; i++){
            printf("%s ", C[i].name);
        }
        printf("\n");
    }
    else {
        if( (0==(strcmp(tokens[1], "fin"))) || (0==(strcmp(tokens[1], "salir"))) || (0==(strcmp(tokens[1], "bye")))){
            if(tokens[2]!=NULL){
                printf("Too many arguments\n");
                return;
            }
            printf("This command ends the shell\n");
        }

        else if((0==(strcmp(tokens[1], "autores")))){
            printf("The command %s gives info on the authors of the code, ", tokens[1]);

            if(tokens[3]!=NULL){
                printf("Too many arguments\n");
                return;
            }

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

            if(tokens[3]!=NULL){
                printf("Too many arguments\n");
                return;
            }

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

            if(tokens[3]!=NULL){
                printf("Too many arguments\n");
                return;
            }

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

        else if((0==(strcmp(tokens[1], "infosis"))))
            printf("the command %s gives info about the current system, there are no variations of this command\n", tokens[1]);

        if(tokens[2]!=NULL){
            printf("Too many arguments\n");
            return;
        }

        else if((0==(strcmp(tokens[1], "comando"))))
            printf("this command executes the command on the list with the wanted position\n");

        if(tokens[2]!=NULL){
            printf("Too many arguments\n");
            return;
        }

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
            printf("The command %s cannot be found\n", tokens[1]);
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
}

void cmd_fin(char **tokens) {
    exit(1);
}

void processInput(char **tokens, char str[]) {

    int i;
    struct tNode node;

    if (tokens[0] == NULL_COMMAND) return;

    for (i = 0 ; C[i].name != NULL ; i++){
        if (!strcmp(tokens[0], C[i].name)) {
            node.next = NULL_COMMAND;
            //strcpy(str, node.data.command);

            if (i > 1)
                insertItem(node, &list);

            if ((i != 2) && (i != 3)  && (i != 11) && (i != 9) && (tokens[2] != NULL_COMMAND))
                printf("Too many arguments\n");
            else
                (C[i].func)(tokens);

            tokens[1] = tokens[2] = tokens[3] = NULL_COMMAND;
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
    char delim[] = " \t\a\r\n";
    char *ptr = strtok(strc, delim);
    int i=0;

    while(ptr != NULL){
        tokens[i] = ptr;
        i++;
        ptr = strtok(NULL, delim);
    }
    return **tokens;
}

void cmd_comando(char **tokens) {

    int N;
    tCommand_pos i = first(list);

    if (tokens[1] != NULL_COMMAND) {

        N = (int) strtol(tokens[1], tokens, 0);

        if (N > 0) {

            for (int j = 1; j < N; ++j) {
                if (i == NULL_COMMAND) break;
                i = next(i, list);
            }

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
        printf("Insert the number of the command\n");
}


int main(){
    char str[MAXLINE];
    char **tokens [MAXLINE];
    createEmptyList(&list);

    while(1) {
        printf("*) ");
        fgets(str, MAXLINE, stdin);
        if (strcmp(str,"\n")) {
            str[strcspn(str, "\n")] = 0;
            splitString(str, tokens);
            processInput(tokens, str);
        }
        memset(str, '0', MAXLINE);
    }

    return 0;
}