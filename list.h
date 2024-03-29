#ifndef SHELL_HIST_H
#define SHELL_HIST_H

#include <stdbool.h>
#include <stdlib.h>

#define LENGTH_LIMIT 1024
#define NULL_TEXT NULL

typedef struct tText{
    char text[LENGTH_LIMIT];
} tText;

struct tMem {
    void * address;
    tText file;
    int key;
    int size;
    tText date;
};

struct tPro {
    pid_t pid;
    tText user;
    tText state;
    int terminatedBy;
    tText priLineTime;
};

//definition of the nodes:
typedef struct tNode * tPos; //position is a pointer to a node
struct tNode { //nodes are structures composed of data and a pointer to the next
    tText data; //data in the node
    struct tMem mem;
    struct tPro pro;
    tPos next; //pointer to next
};
typedef tPos tList;

// headers of functions:
bool isEmptyList (tList l); //it is checked if the pointer ot the first element is null or not, if it is, the list is empty
tPos last(tList list); //returns the last element of the list
void createEmptyList (tList *list); //creates a list that doesn't have items
tPos first(tList list); //returns the position of the first element of a non-empty list
tPos next (tPos pos, tList list); //returns the position of the following element of an item in a given position of a list
bool insertItem(struct tNode node, tList *list); // inserts item with given contents
void deleteAtPosition(tPos pos, tList *list); //deletes item from the list at a given position
void updateProcess(tPos pos, tList *list, char *state, int terminatedBy); //updates a process' information
struct tNode getItem(tPos pos, tList list); //returns the contents of an element at a given position

#endif //SHELL_HIST_H