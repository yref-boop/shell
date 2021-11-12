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
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/wait.h>

#define MAXLINE 1024
tHist list;

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
void cmd_malloc(char **);
void cmd_mmap(char **);
void cmd_shared(char **);
void cmd_dealloc(char **);
void cmd_memoria(char **);
void cmd_volcarmem(char **);
void cmd_llenarmem(char **);
void cmd_recursiva(char **);


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
        {"malloc", cmd_malloc},
        {"mmap", cmd_mmap},
        {"shared", cmd_shared},
        {"dealloc", cmd_dealloc},
        {"memoria", cmd_memoria},
        {"volcarmem", cmd_volcarmem},
        {"llenarmem", cmd_llenarmem},
        {"recursiva", cmd_recursiva},
        {"fin",       cmd_fin},
        {"bye",       cmd_fin},
        {"salir",     cmd_fin},
        {NULL,  NULL}
};


void splitString (char *str, char *tokens[]) {
    char *found;
    int i = 0;

    //eliminate the new line char at the end of the string to ease its management
    str[strcspn(str, "\n")] = 0;

    //set the delimiters
    char delim[] = " \t\a\r\n";

    //store all chars that are before any delimiter or end of string in separated tokens (arrays)
    while(((found = strsep(&str, delim))!= NULL)){
        if((strcmp(found, "") != 0)) {
            tokens[i] = found;
            i++;
        }
    }
}

void storeOnList (char * tokens[]) {
    struct tNode node;
    char store[MAXLINE];
    strcpy(store, "");
    int i = 0;

    //create an string (store) with the current tokens
    while(tokens[i]!=NULL){
        strcat(store, tokens[i]);
        strcat(store, " ");
        i++;
    }

    //insert the new string on the list
    strcpy(node.data.command, store);
    insertItem(node, &list);

    //the values on the string are cleared
    memset(store, 0, sizeof store);
}

void processInput(char *tokens[]) {
    //nothing to process
    if (tokens[0]== NULL) return;

    //check for all commands on C struct if there is a match
    for(int i = 0 ;C[i].name != NULL; i++){
        if(strcmp(tokens[0], C[i].name) == 0) {
            storeOnList(tokens);
            (C[i].func)(tokens);
        }
    }
}

//obtain a pointer to a shared memory zone
//if (tam > 0) tries to create it, if (tam = 0) it assumes it exists
void * ObtenerMemoriaShmget (key_t clave, size_t tam) {
    void * p;
    int aux,id,flags = 0777;
    struct shmid_ds s;
    //if tam is not 0, it creates it in exclusive mode
    if (tam) flags = flags | IPC_CREAT | IPC_EXCL;

    ///esta funcion vale para shared y shared -create

    //if (tam = 0) it tries to access to an already created one
    if (clave == IPC_PRIVATE) {errno = EINVAL; return NULL;} //not useful

    if ((id = shmget(clave, tam, flags)) == -1) return (NULL);

    if ((p = shmat(id,NULL,0)) == (void*) -1){
        aux = errno; //if it was created but couldn't be mapped
        if (tam) //it is erased
            shmctl(id,IPC_RMID,NULL);
        errno = aux;
        return (NULL);
    }

    shmctl (id, IPC_STAT, &s);
    return (p); //save in Shared Memory Addresses (p, s.shm_segsz, clave.....)
}

void SharedCreate (char *arg[]) { //arg[2] is the key and arg[3] is the size
    key_t k;
    size_t tam = 0;
    void *p;
    if (arg[2] == NULL || arg[3] == NULL) return;
    //list Shared Memory's addresses

    k = (key_t) atoi(arg[2]);

    if (arg[3] != NULL)
        tam=(size_t) atoll(arg[3]);

    if ((p=ObtenerMemoriaShmget(k, tam)) == NULL)
        perror ("Imposible obtener memoria shmget");
    else
        printf ("Memoria de shmget de clave %d asignada en %p\n", k, p);
}

void * MmapFichero (char * fichero, int protection) {
    int df, map = MAP_PRIVATE, modo = O_RDONLY;
    struct stat s;
    void *p;

    if (protection&PROT_WRITE) modo = O_RDWR;

    if (stat(fichero, &s) == -1 || (df = open(fichero, modo)) == -1)
        return NULL;

    if ((p = mmap (NULL, s.st_size, protection, map, df, 0)) == MAP_FAILED)
        return NULL;

    //save in Map Addresses (p, s.shm_segsz, clave.....)
    return p;
}

void Mmap (char *arg[]) { //arg[0] is the file name and arg[1] is the permissions
    char *perm;
    void *p;
    int protection = 0;

    if (arg[0] == NULL) return; //List Memory Addresses of mmap

    if ((perm = arg[1]) != NULL && strlen(perm) < 4) {
        if (strchr(perm,'r') != NULL) protection |= PROT_READ;
        if (strchr(perm,'w') != NULL) protection |= PROT_WRITE;
        if (strchr(perm,'x') != NULL) protection |= PROT_EXEC;
    }

    if ((p = MmapFichero(arg[0], protection)) == NULL)
        perror ("Imposible mapear fichero");
    else
        printf ("fichero %s mapeado en %p\n", arg[0], p);
}

#define LEERCOMPLETO ((ssize_t) - 1)

ssize_t LeerFichero (char *fich, void *p, ssize_t n) {
    //reads n bytes of the directory fich in p
    ssize_t nleidos, tam = n; //if (n == -1) it reads the complete directory

    int df, aux;
    struct stat s;
    if (stat (fich, &s) == -1 || (df = open(fich, O_RDONLY)) == -1) return ((ssize_t) - 1);

    if (n == LEERCOMPLETO) tam = (ssize_t) s.st_size;

    if ((nleidos = read(df, p, tam)) == -1) {
        aux = errno;
        close(df);
        errno = aux;
        return ((ssize_t) - 1);
    }

    close (df);
    return (nleidos);
}

void SharedDelkey (char *args[]) { //arg[2] points to a str containing the key
    key_t clave;
    int id;
    char *key = args[2];

    if (key == NULL || (clave = (key_t) strtoul(key, NULL, 10)) == IPC_PRIVATE){
        printf(" shared -delkey clave_valida\n");
        return;
    }

    if ((id = shmget(clave, 0, 0666)) == -1) {
        perror("shmget: imposible obtener memoria compartida");
        return;
    }
    if (shmctl(id, IPC_RMID, NULL) == -1)
        perror("shmctl: imposible eliminar memoria compartida\n");
}

void dopmap (void) { //no arguments necessary
    pid_t pid; //executes the extern command pmap
    char elpid[32]; //pid of the current process
    char *argv[3] = {"pmap", elpid, NULL};
    sprintf (elpid, "%d", (int) getpid());

    if ((pid = fork()) == -1) { perror("Imposible crear proceso"); return; }

    if (pid == 0) {
        if (execvp(argv[0], argv) == -1)
            perror("cannot execute pmap");
        exit(1);
    }
    waitpid (pid, NULL, 0);
}


void cmd_malloc(char **tokens) {

    int n, i, *ptr, sum = 0;

    if (tokens[1] == NULL_COMMAND) {
        //show list of malloc
    } else if (!strcmp(tokens[1], "-free")) {
        if (tokens[2] != NULL_COMMAND){

            //memory that we have to allocate
            free(ptr);

        }
    } else {
        ptr = (int*) malloc(n * sizeof(int));

        if(ptr == NULL) {
            printf("Error! memory not allocated.");
            exit(0);
        }
        printf("Enter elements: ");
        for(i = 0; i < n; ++i) {
            scanf("%d", ptr + i);
            sum += *(ptr + i);
        }

        printf("Sum = %d", sum);
    }
}

void cmd_mmap(char **tokens) {

    if (tokens[1] == NULL_COMMAND) {
        //print list of mmap
    } else if (!strcmp(tokens[1],"-free")) {
        //free(ptr);
    } else if (tokens[2] != NULL_COMMAND) {

        int N = 5;
        int *ptr = mmap ( NULL, N*sizeof(int), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 );

        int err = munmap(ptr, 10*sizeof(int));
        if(err != 0){
            printf("UnMapping Failed\n");
            return;
            //-free
        } else {
            //print list of mmap
        }
    } else {
        //int N = 5;

        int N = 5;
        int *ptr = mmap ( NULL, N*sizeof(int), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0 );

        if (tokens[2] == NULL_COMMAND) {
            //ptr
            if(ptr == MAP_FAILED){
                printf("Mapping Failed\n");
                return;
            }
        }
            //fich
        else {
            //ptr
            if(ptr == MAP_FAILED){
                printf("Mapping Failed\n");
                return;
            }
            //fich [perm]
        }

        for(int i=0; i<N; i++)
            ptr[i] = i*10;

        for(int i=0; i<N; i++)
            printf("[%d] ",ptr[i]);

    }
}

void cmd_shared(char **tokens) {

    if (tokens[1] == NULL_COMMAND) {
        //show shared memory list
    } else if (tokens[2] != NULL_COMMAND) {

        if (!strcmp(tokens[1], "-free")) {
            if (tokens[2] != NULL_COMMAND)
                free(tokens[2]);
            //else show shared memory list
        } else if (!strcmp(tokens[1], "-create")) {
            SharedCreate(tokens);
        } else if (!strcmp(tokens[1], "-delKey")) {
            SharedDelkey(tokens);
        }
    }
}

void cmd_dealloc(char **tokens) {

}

void cmd_memoria(char **tokens) {

}

void cmd_volcarmem(char **tokens) {

}

void cmd_llenarmem(char **tokens) {

}

#define SIZE 4096
void doRecursiva (int n) {
    char automatico[SIZE];
    static char estatico[SIZE];
    printf("parametro n:%d en %p\n", n, &n);
    printf("array estatico en:%p \n", estatico);
    printf("array automatico en %p\n", automatico);
    n--;

    if (n > 0) doRecursiva(n); //?
}

void cmd_recursiva(char **tokens) {
    doRecursiva(SIZE);
}

void cmd_comando(char *tokens[]) {
    tCommand_pos N;

    if (tokens[1] == NULL) {
        printf("command not found");
        return;
    }
    if (tokens[2] != NULL) {
        printf("too many arguments");
        return;
    }

    N = (tCommand_pos) (strtol(tokens[1], tokens, 0));

    if (last(list) > N) {
        printf("there is no such command \n");return;}
    else {
        tCommand command = getItem(N, list);
        splitString(command.command, tokens);
        processInput(tokens);
    }
}

void printList(tCommand_pos pos){
    tCommand_pos aux = first(list);
    tCommand node;
    int i = 0;

    while(aux < pos){
        node = getItem (aux, list);
        printf("%i -> %s\n", i, node.command);
        aux = next(aux, list);
        i++;
    }
}

void cmd_hist(char *tokens[]){
    tCommand_pos p = first(list);
    tCommand node;

    if(tokens[1] == NULL) printList(last(list));
    else{
        if(strcmp(tokens[1], "-c") == 0)
            while(list!=NULL_COMMAND) deleteAtPosition(p, &list);
        else {
            p = (tCommand_pos) strtol(tokens[1], tokens, 0);
            printList(p);
        }
    }
}


void cmd_ayuda(char *tokens[]){
    if(tokens[1] == NULL)
        printf("'ayuda cmd' where cmd represents one of the avaliable commands: \n"
               "comando, hist, ayuda, listfich, listdir, borrar, borrarrec, crear, carpeta\n"
               "autores, pid, fecha, infosis, fin, bye, salir");
    else {
        if (strcmp(tokens[1], "comando") == 0) printf("comando [-N]: repeats command number N from history\n");
        if (strcmp(tokens[1], "hist") == 0) printf("hist [-c|-N]: shows the command history, -c erases it\n");
        if (strcmp(tokens[1], "ayuda") == 0) printf("ayuda [cmd]: shows command help\n");

        if (strcmp(tokens[1], "autores") == 0) printf("autores [-n|-l] shows names and logins of the authors\n");
        if (strcmp(tokens[1], "pid") == 0) printf("pid [-p] shows the pid of the shell or its parent process\n");
        if (strcmp(tokens[1], "fecha") == 0) printf("fecha [-d|-h] shows date and / or current time]\n");
        if (strcmp(tokens[1], "infosis") == 0) printf("infosis  shows information about the machine where its executed \n");
        if (strcmp(tokens[1], "fin") == 0) printf("fin stops the shell\n");
        if (strcmp(tokens[1], "bye") == 0) printf("bye stops the shell\n");
        if (strcmp(tokens[1], "salir") == 0) printf("salir stops the shell\n");
    }
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

void cmd_pid(char *tokens[]){
    if (tokens[1]==NULL)
        printf("Shell process  pid: %d\n", getpid());
    else if(strcmp(tokens[1],"-p")==0)
        printf("Parent process: %d\n", getppid());
    else
        printf("Command %s %s not found \n", tokens[0],tokens[1]);
}


void cmd_fecha(char *tokens[]){

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


void cmd_infosis(char *tokens[]){
    struct utsname unameData;
    uname(&unameData);
    //prints all the information
    printf("Name of the system: %s\nName of this node: %s\nCurrent release: %s\nCurrent version: %s\nHardware type: %s\n",
           unameData.sysname, unameData.nodename, unameData.release, unameData.version, unameData.machine);
}

void cmd_fin(char *tokens[]){}


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
            } else free((char**) buf);
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

int main() {
    char str[MAXLINE]; //variable which stores the input
    createEmptyList(&list);  //list needed for command hist
    char *tokens[MAXLINE];

    while (1) {
        printf("*) ");
        fgets(str, MAXLINE, stdin);

        splitString(str, tokens);
        if(tokens[0] == NULL){return 1;}

        processInput(tokens);

        memset(tokens, 0, sizeof tokens);
        memset(str, '0', sizeof(str));
    }
    free(list);
}