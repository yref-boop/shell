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

void show_dir();
void cmd_crear(char **);
void cmd_borrar(char **);
void print_file(bool **, char *);
void cmd_borrarrec(char **);
bool list_detect(bool **, int, char **, bool, bool);
void cmd_listfich(char **);
void dir(char *, bool **);
void recursive(char *, bool **, int);
void cmd_listdir(char **);
void cmd_autores(char **);
void cmd_carpeta(char **);
void cmd_pid(char **);
void cmd_fecha(char **);
void cmd_infosis(char **);
void cmd_ayuda(char **);
void cmd_hist(char **);
void cmd_fin(char **);
void cmd_comando(char **);
tHist list;

struct CMD {
    char *name;
    void(*func)(char**);
};

struct CMD C[]={
        {"comando",   cmd_comando},
        {"hist",      cmd_hist},
        {"ayuda",     cmd_ayuda},
        {"listfich",  cmd_listfich},
        {"listdir",   cmd_listdir},
        {"borrar",    cmd_borrar},
        {"borrarrec", cmd_borrarrec},
        {"crear",     cmd_crear},
        {"carpeta",   cmd_carpeta},
        {"autores",   cmd_autores},
        {"pid",       cmd_pid},
        {"fecha",     cmd_fecha},
        {"infosis",   cmd_infosis},
        {"fin",       cmd_fin},
        {"bye",       cmd_fin},
        {"salir",     cmd_fin},
        {NULL,  NULL}
};

//auxiliary function
//show_dir prints the current directory

void show_dir() {

    char dir[MAXLINE];
    char *a = getcwd(dir, MAXLINE);

    if (a) printf("%s\n", a); //print the directory
    else printf("%s", strerror(errno)); //error
}

//function corresponding to the command crear
//crear creates a file/folder in the current directory

void cmd_crear(char **tokens) {

    if (tokens[1] == NULL) { show_dir(); return; } //no argument prints the current directory

    if (strcmp(tokens[1], "-f") == 0) { //create a file
        if (fopen(tokens[2], "w") == NULL)
            printf("Error while trying to create %s\n%m\n", tokens[2]);
    } else { //create a folder
        if (mkdir(tokens[1], 0777))
            printf("Error while trying to create %s\n%m\n", tokens[1]);
    }
}

//function corresponding to the command borrar
//borrar deletes the files/*empty* folders with the given names in the current directory

void cmd_borrar(char **tokens) {

    if (tokens[1] == NULL) { show_dir(); return; } //no argument prints the current directory

    int token_point = 1;

    while (tokens[token_point] != NULL) { //iterating through all the arguments

        if (remove(tokens[token_point]) == 0) //delete success
            printf("%s was deleted successfully\n", tokens[token_point]);
        else
            printf("%s couldn't be deleted\n", tokens[token_point]); //error

        token_point++;
    }
}

//auxiliary function
//prints the name and all the attributes of the file (or folder)

void print_file(bool *op[], char *tokens) {

    //structs needed for the info of the file/directory:
    struct stat st = {};
    struct passwd *pwd;
    struct group *grp;

    //in case that lstat returns 0, the info was correctly obtained, else, output error and return
    if (lstat(tokens, &st) != 0) {
        printf("Unable to get file properties.\n");
        printf("Please check whether '%s' file exists.\n", tokens);
        return;
    }

    //if -long is not specified
    if (*op[3] == false) {
        //print size and name
        printf("%ld %s\n", st.st_size, tokens);
    } else {
        //print modification_date number_of_links (inode_number) owner group mode size name->file_links_to

        //if -acc was specified: print last access time instead of modification time
        if (*op[5] == true) {
            char buffer[80];
            strftime(buffer, 80, "%Y/%m/%d-%H:%M ", localtime(&st.st_atime));
            printf("%s", buffer);
        } else {
            char buffer[80];
            strftime(buffer, 80, "%Y/%m/%d-%H:%M ", localtime(&st.st_mtime));
            printf("%s", buffer);
        }

        //print number of links and inode number
        printf("%ld ", st.st_nlink);
        printf("(%ld)", st.st_ino);

        //gets an user ID, prints the user if found or the numeric value of the error if it was not
        if ((pwd = getpwuid(st.st_uid)) != NULL)
            printf(" %s ", pwd->pw_name);
        else
            printf(" %d ", st.st_uid);

        //gets the group ID, prints the group if found or the numeric value of the error if it was not
        if ((grp = getgrgid(st.st_gid)) != NULL)
            printf(" %s ", grp->gr_name);
        else
            printf(" %d", st.st_gid);

        //this block of code prints the mode and the permissions of the current file/directory
        printf((S_ISDIR(st.st_mode)) ? "d" : "-");
        printf((st.st_mode & S_IRUSR) ? "r" : "-");
        printf((st.st_mode & S_IWUSR) ? "w" : "-");
        printf((st.st_mode & S_IXUSR) ? "x" : "-");
        printf((st.st_mode & S_IRGRP) ? "r" : "-");
        printf((st.st_mode & S_IWGRP) ? "w" : "-");
        printf((st.st_mode & S_IXGRP) ? "x" : "-");
        printf((st.st_mode & S_IROTH) ? "r" : "-");
        printf((st.st_mode & S_IWOTH) ? "w" : "-");
        printf((st.st_mode & S_IXOTH) ? "x" : "-");

        //if the lstat function returns error value, print its strerror
        if (lstat(tokens, &st) == -1)
            printf("%s", strerror(errno));

        //print size and name
        printf("% ld %s ", st.st_size, tokens);

        //given the option -link and if the current file/directory is a link
        if (*op[4] && S_ISLNK(st.st_mode)) {
            printf("-> ");

            //create an array and initialize it to zero
            char buff[1024];
            memset(buff, 0, sizeof(buff));

            //the if condition checks if readlink gets a correct link when trying to place
            //the contents of tokens inside buff (with 1 less space in size)
            //if it cannot(returns -1), print strerror, else print the link

            if (readlink(tokens, buff, sizeof(buff) - 1) == -1) {
                printf("file_print: %s\n", strerror(errno));
                return;
            } else {
                printf("%s \n", buff);
            }
            //if it is not a link, function already finished, just print \n
        } else printf("\n");
    }
}

//function corresponding to the command borrar
//borrar deletes the files/*empty* folders with the given names in the current directory

void cmd_borrarrec(char **tokens) {

    char path[MAXLINE];
    strcpy(path, tokens[1]);

    if (tokens[1] == NULL) { show_dir(); return; } //no argument prints the current directory
    else {

        DIR *d = opendir(path);
        size_t path_len = strlen(path);
        int r = -1; //to check for errors

        if (d) {
            struct dirent *p;
            r = 0;

            while (!r && (p=readdir(d))) {

                int r2 = -1;
                char *buf;
                size_t len;

                //Ignores "." and ".." directories
                if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) continue;

                len = path_len + strlen(p->d_name) + 2;
                buf = malloc(len);

                if (buf) {
                    struct stat statbuf; //stores info of a file/folder
                    snprintf(buf, len, "%s/%s", path, p->d_name);

                    if (!lstat(buf, &statbuf)) {
                        if (S_ISDIR(statbuf.st_mode)) { //when checking a directory
                            strcpy(tokens[1], buf);
                            free((char **) buf);
                            cmd_borrarrec(tokens); //recurse
                            continue;
                        }
                        else
                            r2 = unlink(buf); //deletes the file
                    }
                    free((char **) buf);
                } else
                    r = r2;
            }
            closedir(d); //closes the directory
        }
        if (!r) r = rmdir(path); //removes the empty folder

        if (r == -1) printf("%s\n", strerror(errno)); //error
    }
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


//auxiliary function
//list_detect checks/modifies the op[] array when executing listfich and listdir

bool list_detect(bool *op[], int arg, char **tokens, bool change, bool isListDir) {

    bool found = false;

    for (int i = 1; i < 7; ++i) {
        switch (i) {
            //if 'change' is true, modify op[] accordingly
            //if 'change' is false, check if the current tokens[arg] is a file or a function option
            case 1: if ((!strcmp(tokens[arg], "-reca")) && isListDir) {if (change) *op[0] = (bool **) true; else found = true;} break;
            case 2: if ((!strcmp(tokens[arg], "-recb")) && isListDir) {if (change) *op[1] = (bool **) true; else found = true;} break;
            case 3: if ((!strcmp(tokens[arg], "-hid")) && isListDir) {if (change) *op[2] = (bool **) true; else found = true;} break;
            case 4: if (!strcmp(tokens[arg], "-long")) {if (change) *op[3] = (bool **) true; else found = true;} break;
            case 5: if (!strcmp(tokens[arg], "-link")) {if (change) *op[4] = (bool **) true; else found = true;} break;
            case 6: if (!strcmp(tokens[arg], "-acc")) {if (change) *op[5] = (bool **) true; else found = true;} break;
            default: break;
        }
    }

    //if 'change' is false, return true if tokens[arg] is a valid option and false if it isn't
    //if 'change' is true, always return true by default
    if (!change) { if (!found) return false; else return true; }
    else
        return true;
}

//function corresponding to the command listfich
//listfich prints all the files/folders of a directory

void cmd_listfich(char **tokens) {

    if (tokens[1] == NULL) { show_dir(); return; } //no argument prints the current directory

    //       -reca  -recb  -hidden  -lo  -li  -ac
    //op[i]      0      1        2    3    4    5

    bool *op[6];

    for (int i = 0; i < 6; ++i) { //set every value of the array to false by default
        op[i] = malloc(sizeof(bool));
        *op[i] = false;
    }

    int arg = 1;

    for (; list_detect(op, arg, tokens, false, false); ++arg) //iterate until there is no -parameter
        list_detect(op, arg, tokens, true, false); //set the appropriate parameter(s) to true

    //arg already has the value of the first file/folder name
    for(; tokens[arg] != NULL; arg++) print_file(op, tokens[arg]); //print the appropriate file/folder
}

//auxiliary function
//processes the non-recursive listdir

void dir(char *directory, bool *op[]) {

    struct stat st; //stores info about the current file

    if (stat(directory, &st) != 0) { //gets the info and checks for errors
        printf("Unable to get file properties.\n");
        printf("Please check whether '%s' file exists.\n", directory);
        return;
    }

    DIR* dir = opendir(directory); //open the directory
    size_t path_len = strlen(directory);

    if (!dir) { //if it is not a directory
        print_file(op, directory); //print the file
    } else {
        printf("*******"); //header
        print_file(op, directory);

        struct dirent *p; //stores the current file/folder

        while ((p = readdir(dir))) { //while there are more files/folders to check
            char *buf;
            size_t len;

            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
                if (*op[2]) print_file(op, p->d_name); //ignore directories '.' & '..' except when -hid is activated
                continue; //restart the loop
            }

            len = path_len + strlen(p->d_name) + 2;
            buf = malloc(len);

            if (buf) {
                struct stat st2;

                snprintf(buf, len, "%s/%s", directory, p->d_name);
                if (!stat(buf, &st2)) print_file(op, buf); //print the name of the file/folder

                free((char**) buf);
            } else free((char**) buf); /////////////////////////////////////
        }
        closedir(dir); //close the directory
    }
}

//auxiliary function
//recursive processes the '-reca' and '-recb' parameters in listdir

void recursive(char *directory, bool *op[], int config) {

    char path[MAXLINE] = "";
    int it = 1; //iteration number: (1->prints all files and folders; 2->goes through the paths inside every folder
    strcpy(path, directory);
    DIR *d = opendir(path); //directory that is worked on
    size_t path_len = strlen(path);

    if (d) { //if the path is accessible

        if (config == 0) { //header when -reca
            printf("*******");
            printf("%s\n", path);
        }

        struct dirent *p;
        p = readdir(d); //first file/folder to work on

        while (it < 3) { //while both iterations are finished

            if (p == NULL) { //when there is no file/folder
                if (it*config == 1) { //head when -recb
                    printf("*******");
                    printf("%s\n", path);
                }
                it ++;
                closedir(d);
                d = opendir(path);
                path_len = strlen(path);
                p = readdir(d);
                continue; //restarts the loop passing to the next iteration with the new path
            }

            char *buf;
            size_t len;

            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
                if ((*op[2]) && (it == 1)) print_file(op, p->d_name); //ignore directories '.' & '..' except when -hid is activated
                p = readdir(d);
                continue; //restart the loop with the new file/folder
            }

            len = path_len + strlen(p->d_name) + 2;
            buf = malloc(len); //buffer to store the current file

            if (buf) {
                struct stat statbuf;
                snprintf(buf, len, "%s/%s", path, p->d_name);

                if (!lstat(buf, &statbuf)) { //info about the current file
                    if (S_ISDIR(statbuf.st_mode)) { //when directory
                        //prints the current folder (-reca->does it in iteration 1;-recb->does it in iteration 2)
                        if (it == 1 + config) print_file(op, buf);
                        //recursions the current folder (-recb->does it in iteration 1;-reca->does it in iteration 2)
                        if (it == 2 - config) { recursive(buf, op, config); }
                    }
                    else
                        //prints the current file (-reca->does it in iteration 1;-recb->does it in iteration 2)
                    if (it == 1 + config) print_file(op, buf);
                }
                free((char **) buf);
            }
            if (!(p = readdir(d))) { //if next file/folder doesn't exist
                if (it*config == 1) { //header when -recb
                    printf("*******");
                    printf("%s\n", path);
                }
                it ++;
                closedir(d);
                d = opendir(path);
                path_len = strlen(path);
                p = readdir(d); //restarts the loop passing to the next iteration with the new path
            }
        }
        closedir(d); //closes the directory
    } else printf("%s\n", strerror(errno)); //error
}

//function corresponding to the command autores
//autores prints both logins and names of the authors, autores -l only logins and autores -n only names

void cmd_listdir(char **tokens) {

    if (tokens[1] == NULL) { show_dir(); return; } //no argument prints the current directory

    int arg = 1;

    //       -reca  -recb  -hidden  -lo  -li  -ac
    //op[i]      0      1        2    3    4    5

    bool *op[6];

    for (int i = 0; i < 6; ++i) { //set every value of the array to false by default
        op[i] = malloc(sizeof(bool));
        *op[i] = false;
    }

    for (; list_detect(op, arg, tokens, false, true); ++arg) //iterate until there is no -parameter
        list_detect(op, arg, tokens, true, true); //set the appropriate parameter(s) to true

    if (*op[0] || *op[1]) { //if listdir is recursive

        if (*op[0]) //recursive (A)
            for(; tokens[arg] != NULL; arg++) recursive(tokens[arg], op, 0);
        else        //recursive (B)
            for(; tokens[arg] != NULL; arg++) recursive(tokens[arg], op, 1);

    } else {
        //arg already has the value of the first file name
        for(; tokens[arg] != NULL; arg++) { dir(tokens[arg], op); }} //execute the default dir function with all the directories

    for (int i = 0; i < 6; ++i) { //set every value of the array to false by default
        free(op[i]);
        //op[i] = NULL;
    }
}

//function corresponding to the command autores
//autores prints both logins and names of the authors, autores -l only logins and autores -n only names

void cmd_autores(char **tokens) {

    if (tokens[1] == NULL)
        printf("Alejandro Fernandez Vazquez    a.fernandez9@udc.es\nYago Fernandez Rego            yago.fernandez.rego@udc.es\n");
    else if (!strcmp(tokens[1], "-l"))
        printf("a.fernandez9@udc.es\nyago.fernandez.rego@udc.es\n");
    else if (!strcmp(tokens[1], "-n"))
        printf("Alejandro Fernandez Vazquez\nYago Fernandez Rego\n");
    else printf("Command %s %s not found\n", tokens[0], tokens[1]);

}

//function corresponding to the command carpeta
//carpeta creates a folder with a given name in the current directory

void cmd_carpeta(char **tokens) {

    if (tokens[1] == NULL)
        show_dir();
    else {
        if (chdir(tokens[1]) == -1)
            printf("%s\n", strerror(errno));
        else
            show_dir();
    }
}

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
    else if (!strcmp(tokens[1], "-d")) //date
        printf("System Date is: %02d/%02d/%04d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
    else if (!strcmp(tokens[1], "-h")) //time
        printf("System Time is: %02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
    else //wrong argument
        printf("Command %s %s not found\n", tokens[0], tokens[1]);
}

//function corresponding to the command infosis
//it gives info on the current machine

void cmd_infosis(char **tokens) {

    struct utsname unameData;
    uname(&unameData);
    //prints all the information
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

//function corresponding to fin
//it ends the code

void cmd_fin(char **tokens) {
    exit(1);
}

//this function that has the already tokenized string as input
//and decides what command functions to execute

void processInput(char **tokens, char str[]) {

    int i;
    struct tNode node;
    char strc[MAXLINE];
    char *strup = strdup(str); //storing a malloced version of str

    //if the input is empty (the \n was previously erased) then do nothing
    if (tokens[0] == NULL_COMMAND) return;

    //for each position on the struct C that contains the commands,
    for (i = 0 ; C[i].name != NULL ; i++) {

        //if there is a match between the token and one of the commands
        if (!strcmp(tokens[0], C[i].name)) {

            //create a node and store the whole string on 'command', in the list that stores the history of the program
            strcpy(node.data.command, strup);

            if (i > 1)
                insertItem(node, &list);

            //since it is impossible that some commands have 3 or more arguments...
            if (((i < 2) || (i > 7)) && (tokens[2] != NULL_COMMAND))
                printf("Too many arguments\n");
            else
                (C[i].func)(tokens); //the struct C is consulted to get the current function

            //since the function is already finished, the tokens are not needed, thus their content is erased
            for (int j = 0; tokens[j] != NULL; ++j)
                tokens[j] = NULL;
            break;
        }
    }

    free(strup);

    //if the struct C has been completely checked without any being the same as the input, then the input is not an implemented command
    if (C[i].name == NULL)
        printf("Command %s not found\n", tokens[0]);
}

//this function is used to split the string (named previously as trocearCadena())
//and to count the number of words written

char splitString(char str[], char **tokens) {

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

//function corresponding to command comando:
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
        printf("No command number has been inserted\n");
}

int main() {
    char str[MAXLINE]; //variable which stores the input
    char **tokens [MAXLINE]; //pointer to pointer of chars: array of arrays that will contain each separated command
    createEmptyList(&list);  //list needed for command hist

    while(1) {
        printf("*) ");
        fgets(str, MAXLINE, stdin); //store input in str
        if (strcmp(str,"\n") != 0) {     //until new line
            str[strcspn(str, "\n")] = 0;//eliminate the new line char from string to ease its management
            splitString(str, (char **) tokens);   //divides the string into each part and stores on tokens
            processInput((char **) tokens, str);  //processes the tokens
        }
        memset(str, '0', MAXLINE);   //erase all contents on str for next iteration
    }
    free(**tokens);
    free(list);
    return 0;
}