#include "list.h"
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
#include <signal.h>
#include <sys/resource.h>

//Alejandro Fernandez Vazquez    a.fernandez9@udc.es
//Yago Fernandez Rego            yago.fernandez.rego@udc.es

#define MAXLINE 1024
#define READCOMPLETE ((ssize_t) - 1)
#define SIZE 4096
tList list;
tList mem;
int forMemoria;
extern char ** environ;
char ** env2;
char str_stderr [MAXLINE] = "standard error";
int stderr_copy;

void cmd_autores(char **);
void cmd_carpeta(char **);
void cmd_pid(char **);
void cmd_fecha(char **);
void cmd_infosis(char **);
void cmd_ayuda(char **);
void cmd_hist(char **);
void cmd_fin(char **);
void cmd_comando(char **);

void cmd_crear(char **);
void cmd_borrar(char **);
void cmd_borrarrec(char **);
void cmd_listfich(char **);
void cmd_listdir(char **);

void cmd_malloc(char **);
void cmd_mmap(char **);
void cmd_shared(char **);
void cmd_dealloc(char **);
void cmd_memoria(char **);
void cmd_volcarmem(char **);
void cmd_llenarmem(char **);
void cmd_es(char **);
void cmd_recursiva(char **);

void cmd_priority(char **);
void cmd_rederr(char **);
void cmd_entorno(char **);
void cmd_mostrarvar(char **);
void cmd_cambiarvar(char **);
void cmd_uid(char **);
void cmd_fork(char **);
void cmd_ejec(char **);
void cmd_ejecpri(char **);
void cmd_fg(char **);
void cmd_fgpri(char **);
void cmd_back(char **);
void cmd_backpri(char **);
void cmd_ejecas(char **);
void cmd_fgas(char **);
void cmd_bgas(char **);
void cmd_listjobs(char **);
void cmd_job(char **);
void cmd_borrarjobs(char **);

struct CMD {
    char *name;
    void (*func) (char**);
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
        {"malloc",    cmd_malloc},
        {"mmap",      cmd_mmap},
        {"shared",    cmd_shared},
        {"dealloc",   cmd_dealloc},
        {"memoria",   cmd_memoria},
        {"volcarmem", cmd_volcarmem},
        {"llenarmem", cmd_llenarmem},
        {"recursiva", cmd_recursiva},
        {"e-s",       cmd_es},
        {"priority",  cmd_priority},
        {"rederr",    cmd_rederr},
        {"entorno",   cmd_entorno},
        {"mostrarvar",cmd_mostrarvar},
        {"cambiarvar",cmd_cambiarvar},
        {"uid",       cmd_uid},
        {"fork",      cmd_fork},
        {"ejec",      cmd_ejec},
        {"ejecpri",   cmd_ejecpri},
        {"fg",        cmd_fg},
        {"fgpri",     cmd_fgpri},
        {"back",      cmd_back},
        {"backpri",   cmd_backpri},
        {"ejecas",    cmd_ejecas},
        {"fgas",      cmd_fgas},
        {"bgas",      cmd_bgas},
        {"listjobs",  cmd_listjobs},
        {"job",       cmd_job},
        {"borrarjobs",cmd_borrarjobs},
        {"fin",       cmd_fin},
        {"bye",       cmd_fin},
        {"salir",     cmd_fin},
        {NULL,  NULL}
};

///*****************************SIGNALS***********************************
struct SEN{
    char *nombre;
    int senal;
};

static struct SEN sigstrnum[]={
        "HUP", SIGHUP,
        "INT", SIGINT,
        "QUIT", SIGQUIT,
        "ILL", SIGILL,
        "TRAP", SIGTRAP,
        "ABRT", SIGABRT,
        "IOT", SIGIOT,
        "BUS", SIGBUS,
        "FPE", SIGFPE,
        "KILL", SIGKILL,
        "USR1", SIGUSR1,
        "SEGV", SIGSEGV,
        "USR2", SIGUSR2,
        "PIPE", SIGPIPE,
        "ALRM", SIGALRM,
        "TERM", SIGTERM,
        "CHLD", SIGCHLD,
        "CONT", SIGCONT,
        "STOP", SIGSTOP,
        "TSTP", SIGTSTP,
        "TTIN", SIGTTIN,
        "TTOU", SIGTTOU,
        "URG", SIGURG,
        "XCPU", SIGXCPU,
        "XFSZ", SIGXFSZ,
        "VTALRM", SIGVTALRM,
        "PROF", SIGPROF,
        "WINCH", SIGWINCH,
        "IO", SIGIO,
        "SYS", SIGSYS,
//signals that are not everywhere
#ifdef SIGPOLL
        "POLL", SIGPOLL,
#endif
#ifdef SIGPWR
        "PWR", SIGPWR,
#endif
#ifdef SIGEMT
        "EMT", SIGEMT,
#endif
#ifdef SIGINFO
        "INFO", SIGINFO,
#endif
#ifdef SIGSTKFLT
        "STKFLT", SIGSTKFLT,
#endif
#ifdef SIGCLD
        "CLD", SIGCLD,
#endif
#ifdef SIGLOST
        "LOST", SIGLOST,
#endif
#ifdef SIGCANCEL
        "CANCEL", SIGCANCEL,
#endif
#ifdef SIGTHAW
        "THAW", SIGTHAW,
#endif
#ifdef SIGFREEZE
        "FREEZE", SIGFREEZE,
#endif
#ifdef SIGLWP
        "LWP", SIGLWP,
#endif
#ifdef SIGWAITING
        "WAITING", SIGWAITING,
#endif
        NULL,-1,
};

int Senal(char *sen) {
    //returns the number of signal from the name
    int i;
    for (i = 0; sigstrnum[i].nombre != NULL; i++)
        if (!strcmp(sen, sigstrnum[i].nombre)) return sigstrnum[i].senal;
    return -1;
}

char *NombreSenal(int sen) {
    //returns the number of signal from the signal
    //when there is not such sig2str
    int i;
    for (i = 0; sigstrnum[i].nombre != NULL; i++)
        if (sen == sigstrnum[i].senal) return sigstrnum[i].nombre;
    return ("SIGUNKNOWN");
}

void splitString(char *str, char *tokens[]) {
    char *found;
    int i = 0;
    if (!strcmp(str, "\n")) return;
    //eliminate the new line char at the end of the string to ease its management
    str[strcspn(str, "\n")] = 0;

    char delim[] = " \t\a\r\n";

    //store all chars that are before any delimiter or end of string in separated tokens (arrays)
    while(((found = strsep(&str, delim)) != NULL)) {
        if((strcmp(found, "") != 0)) {
            tokens[i] = found;
            i++;
        }
    }
}

void storeOnList(char *tokens[]) {
    struct tNode node;
    char store[MAXLINE];
    strcpy(store, "");
    int i = 0;

    //create a string (store) with the current tokens
    while(tokens[i] != NULL) {
        strcat(store, tokens[i]);
        strcat(store, " ");
        i++;
    }

    //insert the new string on the list
    strcpy(node.data.text, store);
    insertItem(node, &list);

    //the values on the string are cleared
    memset(store, 0, sizeof store);
}

bool processInput(char *tokens[]) {
    //nothing to process
    if (tokens[0] == NULL) return false;

    //check for all commands on C struct if there is a match
    for (int i = 0; C[i].name != NULL; i++) {
        if (strcmp(tokens[0], C[i].name) == 0) {
            if (C[i].func == cmd_fin)
                return true;
            if ((C[i].func != cmd_comando) && (C[i].func != cmd_hist))
                storeOnList(tokens);
            (C[i].func)(tokens);
            return false;
        }
    }

    char store[MAXLINE];
    strcpy(store, "");
    int i = 0;

    //create a string (store) with the current tokens
    while(tokens[i] != NULL) {
        strcat(store, tokens[i]);
        strcat(store, " ");
        i++;
    }

    system(store);
    return false;
}

void show_dir() {
    //prints the current directory
    char dir[MAXLINE];
    char *a = getcwd(dir, MAXLINE);
    if (a) printf("%s\n", a); //print the directory
    else printf("%s", strerror(errno)); //error
}

void print_file(const bool op[], char *token) {
    //structs needed for the info of the file/directory:
    struct stat st = {};
    struct passwd *pwd;
    struct group *grp;

    //error on lstat
    if (lstat(token, &st) != 0) {
        printf("Unable to get file properties\n");
        printf("Please check whether '%s' file exists\n", token);
        return;
    }

    if (op[3] == false) {
        //print size and name
        printf("%ld %s\n", st.st_size, token);
    } else {

        //-acc option
        if (op[5] == true) {
            char buffer[80];
            strftime(buffer, 80, "%Y/%m/%d-%H:%M ", localtime(&st.st_atime));
            printf("%s", buffer);
        } else {
            char buffer[80];
            strftime(buffer, 80, "%Y/%m/%d-%H:%M ", localtime(&st.st_mtime));
            printf("%s", buffer);
        }

        //number of links and inode number
        printf("%ld ", st.st_nlink);
        printf("(%ld)", st.st_ino);

        //user ID
        if ((pwd = getpwuid(st.st_uid)) != NULL)
            printf(" %s ", pwd->pw_name);
        else
            printf(" %d ", st.st_uid);

        //group ID
        if ((grp = getgrgid(st.st_gid)) != NULL)
            printf(" %s ", grp->gr_name);
        else
            printf(" %d", st.st_gid);

        //mode and permissions of the current file/directory
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

        if (lstat(token, &st) == -1)
            printf("%s", strerror(errno));

        //print size and name
        printf("% ld %s ", st.st_size, token);

        //option -link
        if (op[4] && S_ISLNK(st.st_mode)) {
            printf("-> ");

            char buff[1024];
            memset(buff, 0, sizeof(buff));

            //check if readlink gets correct link when placing contents of tokens in buff (-1)
            if (readlink(token, buff, sizeof(buff) - 1) == -1) {
                printf("file_print: %s\n", strerror(errno));
                return;
            } else
                printf("%s \n", buff);

        } else printf("\n");
    }
}

bool list_detect(bool op[], int arg, char *tokens[], bool change, bool isListDir) {
    //checks / modifies the op[] array when executing listfich and listdir
    bool found = false;

    for (int i = 1; i < 7; ++i) {
        switch (i) {
            //if 'change' is true, modify op[] accordingly, else, check if the current tokens[arg] is a file or a function option
            case 1: if ((!strcmp(tokens[arg], "-reca")) && isListDir) {if (change) op[0] = (bool **) true; else found = true;} break;
            case 2: if ((!strcmp(tokens[arg], "-recb")) && isListDir) {if (change) op[1] = (bool **) true; else found = true;} break;
            case 3: if ((!strcmp(tokens[arg], "-hid")) && isListDir) {if (change) op[2] = (bool **) true; else found = true;} break;
            case 4: if (!strcmp(tokens[arg], "-long")) {if (change) op[3] = (bool **) true; else found = true;} break;
            case 5: if (!strcmp(tokens[arg], "-link")) {if (change) op[4] = (bool **) true; else found = true;} break;
            case 6: if (!strcmp(tokens[arg], "-acc")) {if (change) op[5] = (bool **) true; else found = true;} break;
            default: break;
        }
    }
    //if 'change' is false, return true if tokens[arg] is a valid option, if 'change' is true, return true
    if (!change) { if (!found) return false; else return true; }
    else return true;
}

void dir(char *directory, bool op[]) {
    //processes the non-recursive listdir

    struct stat st; //stores info about the current file

    if (stat(directory, &st) != 0) { //gets the info and checks for errors
        printf("Unable to get file properties\n");
        printf("Please check whether '%s' file exists\n", directory);
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
                if (op[2]) print_file(op, p->d_name); //ignore directories '.' & '..' except when -hid is activated
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

void borrarrec(char dir[]) {
    char path[MAXLINE];
    char dir2[MAXLINE];
    strcpy(path, dir);
    strcpy(dir2, dir);
    DIR *d = opendir(path);
    size_t path_len = strlen(path);
    int r = -1; //to check for errors

    if (d) {
        struct dirent *p;
        r = 0;

        while (!r && (p = readdir(d))) {

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
                        strcpy(dir2, buf);
                        free((char **) buf);
                        borrarrec(dir2); //recurse
                        continue;
                    }
                    else
                        r = unlink(buf); //deletes the file
                }
                free((char **) buf);
            } else
                r = -1;
        }
        closedir(d); //closes the directory
    }
    if (!r) r = rmdir(path); //removes the empty folder

    if (r == -1) printf("%s\n", strerror(errno)); //error
}

void recursive(char *directory, bool op[], int config) {
    //processes the '-reca' and '-recb' parameters in listdir
    char path[MAXLINE] = "";
    int it = 1; //iteration number: (1->prints all files and folders; 2->goes through the paths inside every folder)
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
                if (op[2] && (it == 1)) print_file(op, p->d_name); //ignore directories '.' & '..' except when -hid is activated
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
                        if (it == 2 - config) recursive(buf, op, config);
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

char * getDate() {
    //returns a string with current time
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);
    return asctime(info);
}

void saveInList(char function[], void * address, int size, char file[], int key, char date[]) {
    struct tNode node;
    //insert the new string on the list
    strcpy(node.data.text, function);
    node.mem.size = size;
    node.mem.address = address;
    strcpy(node.mem.file.text, file);
    node.mem.key = key;
    strcpy(node.mem.date.text, date);
    insertItem(node, &mem);
}

void readInList(struct tNode node, char function[], char size[]) {
    //prints a node of a list and its contents with same size and function
    if (isEmptyList(mem)) return;
    char Size[MAXLINE];
    sprintf(Size, "%d", node.mem.size);
    char nodeFunction[] = {};
    strcpy(nodeFunction, node.data.text);
    strtok(nodeFunction, " ");

    //if "all" is specified, print all functions with all sizes
    if ((!strcmp(nodeFunction, function) || !strcmp("all", function)) && ((!strcmp(size, Size)) || !strcmp("all", size))) {
        printf("%p: size:%d. %s ", node.mem.address, node.mem.size, node.data.text);
        if (strlen(node.mem.file.text) > 0)
            printf("%s (fd:%d) ", node.mem.file.text, node.mem.key);
        else if (node.mem.key > -1)
            printf("(key %d) ", node.mem.key);
        printf("%s", node.mem.date.text);
    }
}

void deleteMalloc(char size[]) {
    //malloc auxiliary function, deletes malloc list
    tPos p = first(mem);
    while(p != NULL) {
        if (getItem(p, mem).mem.size == strtol(size, NULL, 10)) {
            free(getItem(p,mem).mem.address);
            deleteAtPosition(p, &mem);
            printf("malloc in address %p was deallocated", p);
            return;
        }
        p = next(p, mem);
    }
}

void deleteMmap(char name[]) {
    //mmap auxiliary function, deletes mmap list
    tPos p = first(mem);
    while(p != NULL) {
        if (!strcmp(getItem(p, mem).mem.file.text, name)) {
            close(*name);
            deleteAtPosition(p, &mem);
            printf("mmap memory in address %p was deallocated", p);
            return;
        }
        p = next(p, mem);
    }
}

void deleteAddress(char *address) {
    //dealloc auxiliary function, deletes address list
    tPos p = first(mem);
    while(p != NULL) {
        char ad[MAXLINE];
        sprintf(ad, "%p", getItem(p, mem).mem.address);
        if (!strcmp(address, ad)) { deleteAtPosition(p, &mem); return; }
        p = next(p, mem);
    }
}

void printMemoryList() {
    //prints all the contents inside the memory list
    if (isEmptyList(mem)) return;
    tPos p = first(mem);
    do { readInList(getItem(p, mem), "all", "all"); p = next(p, mem); } while(p != NULL);
}

void printMallocList() {
    //prints all mallocs on the memory list
    if (isEmptyList(mem)) return;
    tPos p = first(mem);
    do { readInList(getItem(p, mem), "malloc", "all"); p = next(p, mem); } while(p != NULL);
}

void printMmapList() {
    //prints all maps on the memory list
    if (isEmptyList(mem)) return;
    tPos p = first(mem);
    do { readInList(getItem(p, mem), "mmap", "all"); p = next(p, mem); } while(p != NULL);
}

void printSharedList() {
    //prints all shared on the memory list
    if (isEmptyList(mem)) return;
    tPos p = first(mem);
    do { readInList(getItem(p, mem), "shared", "all"); p = next(p, mem); } while(p != NULL);
}

void * ObtenerMemoriaShmget (key_t clave, size_t tam) {
    //obtain a pointer to a shared memory zone
    //if (tam > 0) tries to create it, if (tam = 0) it assumes it exists
    void * p;
    int aux, id, flags = 0777;
    struct shmid_ds s;
    //if tam is not 0, it creates it in exclusive mode
    if (tam) flags = flags | IPC_CREAT | IPC_EXCL;


    //if (tam = 0) it tries to access to an already created one
    if (clave == IPC_PRIVATE) { errno = EINVAL; return NULL; } //not useful
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

void * MmapFichero (char * fichero, int protection) {
    int df, map = MAP_PRIVATE, modo = O_RDONLY;
    struct stat s;
    void *p;
    if (protection&PROT_WRITE) modo = O_RDWR;
    if (stat(fichero, &s) == -1 || (df = open(fichero, modo)) == -1) return NULL;
    if ((p = mmap (NULL, s.st_size, protection, map, df, 0)) == MAP_FAILED) return NULL;
    saveInList("mmap", p, (int) s.st_size, fichero, df, getDate());
    //save in Map Addresses (p, s.shm_segsz, key...)
    return p;
}

void Mmap (char *tokens[]) {
    char *perm;
    void *p;
    int protection = 0;

    if ((perm = tokens[2]) != NULL && strlen(perm) < 4) {
        if (strchr(perm,'r') != NULL) protection |= PROT_READ;
        if (strchr(perm,'w') != NULL) protection |= PROT_WRITE;
        if (strchr(perm,'x') != NULL) protection |= PROT_EXEC;
    }

    if ((p = MmapFichero(tokens[1], protection)) == NULL)
        printf("Couldn't map file: %s\n", strerror(errno));
    else
        printf ("file %s mapped in %p\n", tokens[1], p);
}

void doRecursiva (int n) {
    //recursive function
    char automatico[SIZE];
    static char estatico[SIZE];
    printf("parameter n:%d in %p\n", n, &n);
    printf("static array in:%p \n", estatico);
    printf("automatic array in %p\n", automatico);
    n--;

    if (n > 0) doRecursiva(n);
}

void SharedCreate (char *tokens[]) {
    key_t k;
    size_t tam = 0;
    void *p;
    if (tokens[3] == NULL) { printSharedList(); return; }

    //list Shared Memory's addresses
    k = (key_t) (int) strtol(tokens[2], NULL, 10);

    if (tokens[3] != NULL) tam = (size_t) strtoll(tokens[3], NULL, 10);
    if ((p = ObtenerMemoriaShmget(k, tam)) == NULL)
        printf("Couldn't obtain memory: %s\n", strerror(errno));
    else {
        printf("Allocated shared memory (key:%d) at %p\n", k, p);
        saveInList("shared memory", p, (int) tam, "", k, getDate());
    }
}

ssize_t ReadFich (char *fich, void *p, ssize_t n) {
    //reads n bytes of the directory fich in p
    ssize_t nRead, tam = n; //if (n == -1) it reads the complete directory
    int fd, aux;
    struct stat s;

    if (stat(fich, &s) == -1 || (fd = open(fich, O_RDONLY)) == -1) {
        printf("Error: %s\n", strerror(errno));
        return (ssize_t) - 1;
    }
    if (n == READCOMPLETE) tam = (ssize_t) s.st_size;
    if ((nRead = read(fd, p, tam)) == -1) {
        aux = errno;
        close(fd);
        errno = aux;
        printf("Error: %s\n", strerror(errno));
        return (ssize_t) - 1;
    }
    printf("success!");
    close (fd);
    return nRead;
}

ssize_t WriteFich (char *fich, void *p, ssize_t n, int overwrite) {
    //reads n bytes of the directory fich in p
    ssize_t nRead, tam = n; //if (n == -1) it reads the complete directory
    int fd, aux;
    struct stat s;

    if (overwrite == 1) {
        if ((fd = open(fich, O_WRONLY | O_TRUNC)) == -1) {
            printf("Error: %s\n", strerror(errno)); return (ssize_t) -1;
        }
    } else {
        if (open(fich, O_WRONLY) != -1) {
            printf("File already exists: try with overwrite: -o\n"); return (ssize_t) -1;
        } else {
            if ((fd = open(fich, O_WRONLY | O_CREAT)) == -1) {
                printf("Error: %s\n", strerror(errno)); return (ssize_t) -1; }
        }
    }

    if (stat(fich, &s) == -1) { printf("Error: %s\n", strerror(errno)); return (ssize_t) -1; }
    if ((nRead = write(fd, p, tam)) == -1) {
        aux = errno;
        close(fd);
        errno = aux;
        printf("Error: %s\n", strerror(errno));
        return (ssize_t) - 1;
    }
    printf("success!");
    close (fd);
    return nRead;
}

void SharedDelkey (char *tokens[]) { //tokens[2] points to a str containing the key
    key_t clave;
    int id;
    char *key = tokens[2];

    if (key == NULL || (clave = (key_t) strtoul(key, NULL, 10)) == IPC_PRIVATE){
        printf(" shared -delkey valid_key\n");
        return;
    }

    if ((id = shmget(clave, 0, 0666)) == -1) {
        printf("Couldn't get shared memory: %s\n", strerror(errno));
        return;
    }
    if (shmctl(id, IPC_RMID, NULL) == -1)
        printf("Couldn't delete shared memory: %s\n", strerror(errno));
    else
        printf("Key %s removed from the system\n", tokens[2]);
}

void SharedFree (char key[]) { //tokens[2] points to a str containing the key
    tPos p = first(mem);
    while(p != NULL) {
        if (getItem(p, mem).mem.key == strtol(key, NULL, 10)) {
            if (shmdt(getItem(p, mem).mem.address) == -1)
                printf("Couldn't detach shared memory: %s\n", strerror(errno));
            printf("Shared memory block at %p (key %d) has been deallocated\n",
                   getItem(p, mem).mem.address, getItem(p, mem).mem.key);
            deleteAtPosition(p, &mem);
            return;
        }
        p = next(p, mem);
    }
}

void dopmap (void) { //no arguments necessary
    pid_t pid; //executes the extern command pmap
    char elpid[32]; //pid of the current process
    char *argv[3] = {"pmap", elpid, NULL};
    sprintf (elpid, "%d", (int) getpid());

    if ((pid = fork()) == -1) { printf("Couldn't create process: %s\n", strerror(errno)); return; }
    if (pid == 0) {
        if (execvp(argv[0], argv) == -1)
            printf("Cannot execute pmap: %s\n", strerror(errno));
        exit(1);
    }
    waitpid (pid, NULL, 0);
}

void doblocks() {
    //prints the memory stored on blocks
    printf("MEMORY ADDRESS LIST:\n");
    printMemoryList();
}

void dovars() {
    //gets and prints the variables stored in memory
    printf("GLOBAL VARIABLES:\n");
    printf("Address of list: %p\n", &list);
    printf("Address of mem: %p\n", &mem);
    printf("Address of forMemoria: %p\n", &forMemoria);

    static int a = 7;
    static char b = 'b';
    static long int c[] = {20};

    printf("STATIC VARIABLES:\n");
    printf("Address of a: %p\n", &a);
    printf("Address of b: %p\n", &b);
    printf("Address of c: %p\n", &c);

    char d = 75;
    char e = 'e';
    double f = 74823;

    printf("LOCAL VARIABLES:\n");
    printf("Address of d: %p\n", &d);
    printf("Address of e: %p\n", &e);
    printf("Address of f: %p\n", &f);
}

void dofuncs() {
    //gets and prints the functions stored in memory
    printf("FUNCTION ADDRESSES:\n");
    printf("Address of domap: %p\n", dopmap);
    printf("Address of doblocks: %p\n", doblocks);
    printf("Address of dovars: %p\n", dovars);
    printf("Address of printf: %p\n", printf);
    printf("Address of malloc: %p\n", malloc);
    printf("Address of strcmp: %p\n", strcmp);
}

void printHist(tPos pos) {
    //prints the contents of the given list
    tPos aux = first(list);
    struct tNode node;
    int i = 0;

    while(aux < pos){
        node = getItem (aux, list);
        printf("%i -> %s\n", i, node.data.text);
        aux = next(aux, list);
        i++;
    }
    node = getItem (aux, list);
    printf("%i -> %s\n", i, node.data.text);
}

void * charToVoid(char string[]) {
    //turns strings into null tokens
    void * output = (void *) (long int) strtol(string, NULL, 16);
    return output;
}

void ESread(char *tokens[]) {
    int n = -1;
    void * address = charToVoid(tokens[3]);
    if (tokens[4] != NULL)
        n = (int) strtol(tokens[4], NULL, 10);
    if (ReadFich(tokens[2], address, n) < 0) printf("Error: %s\n", strerror(errno));
}

void ESwrite(char *tokens[]) {
    int o = 0, n = 0;
    if (!strcmp(tokens[2], "-o")) o = 1;
    void * address = charToVoid(tokens[3 + o]);
    if (tokens[4 + o] != NULL)
        n = (int) strtol(tokens[4 + o], NULL, 10);
    if (WriteFich(tokens[2 + o], address, n, o) < 0) printf("Error: %s\n", strerror(errno));
}

void cmd_carpeta(char *tokens[]) {
    if (tokens[1] == NULL) show_dir();
    else {
        //changes directory and checks for errors
        if (chdir(tokens[1]) == -1) printf("%s\n", strerror(errno));
        else show_dir();
    }
}

void cmd_malloc(char *tokens[]) {
    void * *ptr;

    if (tokens[1] == NULL_TEXT) { printMallocList(); return; }
    else if (!strcmp(tokens[1], "-free")) {
        if (tokens[2] != NULL_TEXT) {
            deleteMalloc(tokens[2]);
        } else printMallocList();
    } else {
        ptr = (void *) malloc(strtol(tokens[1], NULL, 10) * sizeof(void *));
        if (ptr == NULL) { printf("Error! memory not allocated.\n"); return; } else
            printf("allocated %s at %p\n", tokens[1], ptr);
        saveInList("malloc", ptr, (int) strtol(tokens[1], NULL, 10), "", 0, getDate());
    }
}

void cmd_mmap(char *tokens[]) {
    if (tokens[1] == NULL_TEXT) { printMmapList(); return; }
    else if (!strcmp(tokens[1], "-free")) {
        if (tokens[2] != NULL_TEXT) deleteMmap(tokens[2]);
        else printMmapList();
    } else Mmap(tokens);
}

void cmd_shared(char *tokens[]) {
    if (tokens[1] == NULL_TEXT) { printSharedList(); return; }
    else if (tokens[2] != NULL_TEXT) {
        if (!strcmp(tokens[1], "-free")) SharedFree(tokens[2]);
        else if (!strcmp(tokens[1], "-create")) SharedCreate(tokens);
        else if (!strcmp(tokens[1], "-delkey")) SharedDelkey(tokens);
        else printSharedList();
    }
}

void cmd_dealloc(char *tokens[]) {
    if (tokens[1] == NULL_TEXT) { printMemoryList(); return; }
        //calls auxiliary functions
    else if (tokens[2] != NULL_TEXT) {
        if (!strcmp(tokens[1], "-malloc")) deleteMalloc(tokens[2]);
        else if (!strcmp(tokens[1], "-shared")) SharedFree(tokens[2]);
        else if (!strcmp(tokens[1], "-mmap")) deleteMmap(tokens[2]);
    } else deleteAddress(tokens[1]);
}

void cmd_memoria(char *tokens[]) {
    bool all = false;
    //calls auxiliary functions and checks if all is stated on the command
    if (tokens[1] == NULL_TEXT || !strcmp(tokens[1], "-all")) all = true;
    else if (!strcmp(tokens[1], "-pmap")) { dopmap(); return; }
    if (all || !strcmp(tokens[1], "-blocks")) doblocks();
    if (all || !strcmp(tokens[1], "-vars")) dovars();
    if (all || !strcmp(tokens[1], "-funcs")) dofuncs();
}

void cmd_volcarmem(char *tokens[]) {
    int i, j, k, length, lineLength;
    length = lineLength = 25;

    void * address = charToVoid(tokens[1]);
    char * txt = (char *) address;

    //set length if necessary
    if (tokens[2] != NULL) length = (int) strtol(tokens[2], NULL, 10);

    //for loops that print both text and values
    for (i = 0; i < length && txt[i] != '\0'; i += lineLength) {
        for (j = i; j < length && j - i < lineLength && txt[j] != '\0'; j++) {
            if (txt[j] != '\n') printf(" %c ", txt[j]);
            else printf("   ");
        }
        printf("\n");
        for (k = i; k < length && k - i < lineLength && txt[k] != '\0'; k++)
            printf("%.2x ", txt[k]);
        printf("\n");
    }
}

char charToASCII(char string[]) {
    //returns the ascii value given several possible formats
    char output;
    if (string[1] == 'x')
        output = (char) strtoul(string, NULL, 16);
    else {
        output = (char) strtol(string, NULL, 10);
        if (output == 0) output = string[0];
    }
    return output;
}

void cmd_llenarmem(char *tokens[]) {
    char byte = 65;
    int cont = 128;
    void * address = charToVoid(tokens[1]);

    //select the options
    if (tokens[2] != NULL) {
        if (tokens[3] != NULL) { cont = (int) strtol(tokens[2], NULL, 10);
            byte = charToASCII(tokens[3]); }
        else cont = (int) strtol(tokens[2], NULL, 10);
    }
    //fill memory accordingly
    memset(address, byte, (size_t) cont);
}

void cmd_recursiva(char *tokens[]) {
    if (tokens[1] != NULL)
        doRecursiva((int) strtol(tokens[1], NULL, 10));
}

void cmd_comando(char *tokens[]) {
    if(isEmptyList(list)) { printf("list is empty\n"); return; }
    tPos N = first(list);
    tPos aux  = N;
    int n_aux = 0;

    if (tokens[1] == NULL) { printf("command not found\n"); return; }
    if (tokens[2] != NULL) { printf("too many arguments\n"); return; }

    int n = (int)(strtol(tokens[1], tokens, 0));
    while (aux < last(list)) { aux = next(aux, list); n_aux++; }

    if (n_aux < n) {
        printf("there is no such command\n"); return; }
    else {
        while (n_aux > 0) { N = next(N, list); n_aux--; }
        //get data and execute the command
        struct tNode command = getItem(N, list);
        memset(tokens, 0, MAXLINE);
        splitString(command.data.text, tokens);
        processInput(tokens);
    }
}

void cmd_hist(char *tokens[]) {
    if(isEmptyList(list)){printf("list is empty\n");return;}
    tPos p = first(list);

    if(tokens[1] == NULL) printHist(last(list));
    else {
        //clear list
        if(strcmp(tokens[1], "-c") == 0)
            while(list!=NULL){
                p = last(list);
                deleteAtPosition(p, &list);
            }
            //print until given number
        else if((int)strtol(tokens[1], tokens, 0) > 0){
            int i = (int)(strtol(tokens[1], tokens, 0)-1);
            while (i > 0) {
                if (p == last(list)) { printf("value is too big\n"); return; }
                p = next(p, list);
                i--;
            }
            printHist(p);
        }
    }
}

void cmd_ayuda(char *tokens[]) {
    //prints info on commands
    if (tokens[1] == NULL)
        printf("'ayuda cmd' where cmd represents one of the avaliable commands:\n"
               "fin salir bye fecha pid autores hist comando carpeta infosis ayuda crear borrar borrarrec \n"
               "listfich listdir recursiva e-s volcarmem llenarmem dealloc malloc mmap shared memoria\n");
    else {
        if (strcmp(tokens[1], "comando") == 0) printf("comando [-N]: repeats command number N from history\n");
        if (strcmp(tokens[1], "hist") == 0) printf("hist [-c|-N]: shows the command history, -c erases it\n");
        if (strcmp(tokens[1], "ayuda") == 0) printf("ayuda [cmd]: shows command help\n");
        if (strcmp(tokens[1], "autores") == 0) printf("autores [-n|-l] shows names and logins of the authors\n");
        if (strcmp(tokens[1], "pid") == 0) printf("pid [-p] shows the pid of the shell or its parent process\n");
        if (strcmp(tokens[1], "fecha") == 0) printf("fecha [-d|-h] shows date and / or current time]\n");
        if (strcmp(tokens[1], "infosis") == 0)printf("infosis  shows information about the machine where its executed \n");
        if (strcmp(tokens[1], "fin") == 0) printf("fin stops the shell\n");
        if (strcmp(tokens[1], "bye") == 0) printf("bye stops the shell\n");
        if (strcmp(tokens[1], "salir") == 0) printf("salir stops the shell\n");
        if (strcmp(tokens[1], "crear") == 0) printf("crear [-f] [name]: creates a file or directory\n");
        if (strcmp(tokens[1], "borrar") == 0) printf("borrar [name1 name2 ...]: deletes files or empty directories\n");
        if (strcmp(tokens[1], "borrarrec") == 0)printf("borrarrec [name1 name2 ...]: deletes files or non-empty directories\\n");
        if (strcmp(tokens[1], "listfich") == 0) printf("listfich [-long][-link][-acc] name2 n1 n2...: lists files\n");
        if (strcmp(tokens[1], "listdir") == 0)printf("listdir [-long][-link][-acc][-reca][-recb][-hid] name2 n1 n2...: lists files inside directories\n");
        if (strcmp(tokens[1], "recursiva") == 0) printf("recursiva [n]: calls ""recursiva"" function n times\n");
        if (strcmp(tokens[1], "e-s") == 0)printf("e-s [read|write][-o] fiche addr cont: reads or writes the file specified\n");
        if (strcmp(tokens[1], "volcarmem") == 0)printf("volcarmem addr cont: dumps contents from addr onto the terminal\n");
        if (strcmp(tokens[1], "llenarmem") == 0) printf("llenarmem addr cont byte: fills memory from addr with byte\n");
        if (strcmp(tokens[1], "dealloc") == 0)printf("dealloc -malloc|-shared...: deallocates a block of memory assiged with parameter\n");
        if (strcmp(tokens[1], "malloc") == 0) printf("malloc [-free] tam: allocs or deallocs memory on the program\n");
        if (strcmp(tokens[1], "mmap") == 0)printf("mmap [-free] fich prms: maps or unmaps files on program's memory space\n");
        if (strcmp(tokens[1], "shared") == 0)printf("shared [-free|-create|-delkey]cl tam: assigns or deasigns shared memory\n");
        if (strcmp(tokens[1], "memoria") == 0)printf("memoria [-blocks|-funcs|-vars|-all|-pmap] : shows memory info\n");
    }
}

void cmd_autores(char *tokens[]) {
    //print autores
    if (tokens[1] == NULL) printf("Alejandro Fernandez Vazquez    a.fernandez9@udc.es\nYago Fernandez Rego            yago.fernandez.rego@udc.es\n");
    else if (!strcmp(tokens[1], "-l")) printf("a.fernandez9@udc.es\nyago.fernandez.rego@udc.es\n");
    else if (!strcmp(tokens[1], "-n")) printf("Alejandro Fernandez Vazquez\nYago Fernandez Rego\n");
    else printf("Command %s %s not found\n", tokens[0], tokens[1]);
}

void cmd_pid(char *tokens[]) {
    //print pid or parent
    if (tokens[1] == NULL) printf("Shell process  pid: %d\n", getpid());
    else if (strcmp(tokens[1], "-p") == 0) printf("Parent process: %d\n", getppid());
    else printf("Command %s %s not found \n", tokens[0],tokens[1]);
}

void cmd_fecha(char *tokens[]) {
    //print date
    time_t T = time(NULL);
    struct tm tm = *localtime(&T);

    if (tokens[1] == NULL) {
        printf("System Date is: %02d/%02d/%04d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
        printf("System Time is: %02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
    else if (!strcmp(tokens[1], "-d")) printf("System Date is: %02d/%02d/%04d\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900); //date
    else if (!strcmp(tokens[1], "-h")) printf("System Time is: %02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec); //time
    else printf("Command %s %s not found\n", tokens[0], tokens[1]);
}

void cmd_infosis(char *tokens[]) {
    struct utsname unameData;
    uname(&unameData);
    //prints all the information
    printf("Name of the system: %s\nName of this node: %s\nCurrent release: %s\nCurrent version: %s\nHardware type: %s\n",
           unameData.sysname, unameData.nodename, unameData.release, unameData.version, unameData.machine);
}

void cmd_fin(char *tokens[]) {}

void cmd_crear(char *tokens[]) {
    if (tokens[1] == NULL) { show_dir(); return; } //no argument prints the current directory
    if (strcmp(tokens[1], "-f") == 0) { //create a file
        if (fopen(tokens[2], "w") == NULL)
            printf("Error while trying to create %s\n%m\n", tokens[2]);
    } else if (mkdir(tokens[1], 0777))
        printf("Error while trying to create %s\n%m\n", tokens[1]); //create a folder
}

void cmd_borrar(char *tokens[]) {
    if (tokens[1] == NULL) { show_dir(); return; } //no argument prints the current directory
    int token_point = 1;
    while (tokens[token_point] != NULL) { //iterating through all the arguments
        if (remove(tokens[token_point]) == 0) //delete success
            printf("%s was deleted successfully\n", tokens[token_point]);
        else printf("%s couldn't be deleted\n", tokens[token_point]); //error
        token_point++;
    }
}

void cmd_borrarrec(char *tokens[]) {
    if (tokens[1] == NULL) { show_dir(); return; } //no argument prints the current directory
    for (int i = 1; tokens[i] != NULL; i++)
        borrarrec(tokens[i]);
}

void cmd_listfich(char *tokens[]) {
    if (tokens[1] == NULL) { show_dir(); return; } //no argument prints the current directory

    //       -reca  -recb  -hidden  -lo  -li  -ac
    //op[i]      0      1        2    3    4    5

    bool op[6] = {false};
    int arg = 1;

    for (; list_detect(op, arg, tokens, false, false); ++arg) //iterate until there is no -parameter
        list_detect(op, arg, tokens, true, false); //set the appropriate parameter(s) to true

    //arg already has the value of the first file/folder name
    for(; tokens[arg] != NULL; arg++) print_file(op, tokens[arg]); //print the appropriate file/folder
}

void cmd_listdir(char *tokens[]) {
    if (tokens[1] == NULL) { show_dir(); return; } //no argument prints the current directory

    int arg = 1;

    //       -reca  -recb  -hidden  -lo  -li  -ac
    //op[i]      0      1        2    3    4    5

    bool op[6] = {false};

    for (; list_detect(op, arg, tokens, false, true); ++arg) //iterate until there is no -parameter
        list_detect(op, arg, tokens, true, true); //set the appropriate parameter(s) to true

    if (op[0] || op[1]) { //if listdir is recursive

        if (op[0]) //recursive (A)
            for(; tokens[arg] != NULL; arg++) recursive(tokens[arg], op, 0);
        else        //recursive (B)
            for(; tokens[arg] != NULL; arg++) recursive(tokens[arg], op, 1);

    } else {
        //arg already has the value of the first file name
        for(; tokens[arg] != NULL; arg++) { dir(tokens[arg], op); } //execute the default dir function with all the directories
    }
}

void cmd_es(char *tokens[]) {
    if (!strcmp(tokens[1], "read")) ESread(tokens);
    else if (!strcmp(tokens[1], "write")) ESwrite(tokens);
}

int SeekVariable (char *var, char *e[]) {
    int pos = 0;
    char aux[MAXLINE];
    strcpy(aux,var);
    strcat(aux, "=");
    while (e[pos] != NULL)
        if (!strncmp(e[pos], aux, strlen(aux))) return pos;
        else pos++;
    errno = ENOENT; //there is not such variable
    return -1;
}

void MostrarEntorno(char **entorno, char *nombre_entorno) {
    int i = 0;
    while (entorno[i] != NULL) {
        printf ("%p->%s[%d]=(%p) %s\n", &entorno[i], nombre_entorno, i, entorno[i], entorno[i]);
        i++;
    }
}

void ShowVariable(char **entorno, char *var) {
    char * getEnv = getenv(var);
    int busVar = SeekVariable(var, entorno);

    if (busVar != -1) {
        printf("%s (%p)<-(%p)\n", entorno[busVar], entorno[busVar], &entorno[busVar]);
        printf("%s (%p)<-(%p)\n", getEnv, getEnv, &getEnv);
    } else
        printf("Error: Couldn't find variable %s\n", var);
}

int ChangeVariable(char *var, char *valor, char *e[]) {
    int pos;
    char *aux;
    if ((pos = SeekVariable(var, e)) == -1) return -1;
    if ((aux = (char *) malloc(strlen(var) + strlen(valor) + 2)) == NULL) return -1;
    strcpy(aux, var);
    strcat(aux, "=");
    strcat(aux, valor);
    e[pos] = aux;
    return pos;
}

char * UserName (uid_t uid) {
    struct passwd *p;
    if ((p = getpwuid(uid)) == NULL) return " ??????";
    return p->pw_name;
}

uid_t UserUID (char *nombre) {
    struct passwd *p;
    if ((p = getpwnam(nombre)) == NULL) return (uid_t) - 1;
    return p->pw_uid;
}

void ShowUIDs (void) {
    uid_t real = getuid(), efec = geteuid();
    printf("Real credential: %d, (%s)\n", real, UserName(real));
    printf("Effective credential: %d, (%s)\n", efec, UserName(efec));
}

void ChangeUIDlogin (char *login) {
    uid_t uid;
    if ((uid = UserUID(login)) == (uid_t) - 1) { printf("Login not valid: %s\n", login); return; }
    if (setuid(uid) == .1) printf("Unable to create credential: %s\n", strerror(errno));
}

void cmd_priority (char *tokens[]) {
    int which = PRIO_PROCESS;
    pid_t pid;
    if (tokens [1] == NULL) pid = getpid();
    else pid = (pid_t) strtol(tokens[1], NULL, 10);
    if(tokens [2] == NULL) printf("%d\n", getpriority(which, pid));
    else {
        int priority = (int) strtol(tokens[2], NULL, 10);
        if (!(setpriority(which, pid, priority))) printf("\"the new priority has been set\\n");
        else printf("the priority could not be set\n");
    }
}

void cmd_rederr (char *tokens[]) {
    fprintf( stderr, "my %s has %d chars\n", "string format", 30);
    if ( tokens[1] == NULL ) printf("standard error at: %s\n", str_stderr);
    else {
        if (strcmp(tokens[1], "-reset") == 0){
            if (dup2(stderr_copy,STDERR_FILENO) == -1) printf("could not allocate standard error to given file \n");
            else printf("standard error reallocated correctly\n");
            strcpy(str_stderr, "standard error");
        }
        else{
            int fderr = open (tokens[1], (O_RDWR | O_CREAT), S_IWGRP);
            if (dup2(fderr,STDERR_FILENO) == -1) printf("could not allocate standard error to given file \n");
            else printf("standard error reallocated correctly\n");
            close(fderr);
            strcpy(str_stderr, tokens[1]);
        }
    }
    fprintf( stderr, "my %s has %d chars\n", "string format", 30);
}

void cmd_entorno (char *tokens[]) {
    if (tokens[1] == NULL) MostrarEntorno(env2, "shell");
    else if (!strcmp(tokens[1], "-environ")) MostrarEntorno(environ, "shell");
    else if (!strcmp(tokens[1], "-addr")) {
        MostrarEntorno(env2, "shell");
        MostrarEntorno(environ, "shell");
    }
}

void cmd_mostrarvar (char *tokens[]) {
    if (tokens[1] == NULL) MostrarEntorno(env2, "shell");
    else ShowVariable(env2, tokens[1]);
}

void cmd_cambiarvar (char *tokens[]) {
    if (!strcmp(tokens[1], "-a")) ChangeVariable(tokens[2], tokens[3], env2);
    else if (!strcmp(tokens[1], "-e")) ChangeVariable(tokens[2], tokens[3], environ);
    else if (!strcmp(tokens[1], "-p")) {
        char aux[MAXLINE];
        strcpy(aux, tokens[2]);
        strcat(aux, "=");
        strcat(aux, tokens[3]);
        putenv(aux);
    } else printf("Choose a valid option (-a/-e/-p)");
}

void cmd_uid (char *tokens[]) {
    if (tokens[1] == NULL || tokens[2] == NULL || !strcmp(tokens[1], "-get")) ShowUIDs();
    else if (!strcmp(tokens[1], "-set") && !strcmp(tokens[2], "-l")) ChangeUIDlogin(tokens[3]);
    else printf("Error: choose a valid option (-get|-set -l id)\n");
}

void cmd_fork (char *tokens[]) {
    if (fork() == 1) exit(0); //terminate child
    else wait(NULL); //reaping parent
}

void cmd_ejec (char *tokens[]) {

}

void cmd_ejecpri (char *tokens[]) {}

void cmd_fg (char *tokens[]) {}

void cmd_fgpri (char *tokens[]) {}

void cmd_back (char *tokens[]) {}

void cmd_backpri (char *tokens[]) {}

void cmd_ejecas (char *tokens[]) {}

void cmd_fgas (char *tokens[]) {}

void cmd_bgas (char *tokens[]) {}

void cmd_listjobs (char *tokens[]) {}

void cmd_job (char *tokens[]) {}

void cmd_borrarjobs (char *tokens[]) {}

int main(int argc, char *argv[], char *env[]) {
    char str[MAXLINE];      //variable which stores the input
    createEmptyList(&list); //list needed for command hist
    createEmptyList(&mem);  //list needed for allocated memory lists
    char *tokens[MAXLINE];
    stderr_copy = dup(STDERR_FILENO);
    bool finish = false;    //bool for the loop
    env2 = env;

    while (!finish) {
        printf("*) ");
        fgets(str, MAXLINE, stdin);
        splitString(str, tokens);
        finish = processInput(tokens);
        memset(tokens, 0, sizeof tokens);
        memset(str, '0', sizeof(str));
    }
    close(stderr_copy);
}