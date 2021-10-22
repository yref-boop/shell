#ifndef SHELL_HIST_H
#define SHELL_HIST_H

#include <stdbool.h>
#include <stdlib.h>

#define COMMAND_LENGTH_LIMIT 1024
#define NULL_COMMAND NULL

typedef struct tCommand{
    char command[COMMAND_LENGTH_LIMIT];
} tCommand;

//definition of the nodes:
typedef struct tNode * tCommand_pos; //position is a pointer to a node
struct tNode { //nodes are structures composed of data and a pointer to the next
    tCommand data; //data in the node
    tCommand_pos next; //pointer to next
};
typedef tCommand_pos tHist;

// headers of functions:
void createEmptyList (tHist* list); //creates a list that doesn't have items
tCommand_pos first(tHist list); //returns the position of the first element of a non-empty list
tCommand_pos next (tCommand_pos pos, tHist list); //returns the position of the following element of an item in a given position of a list
tCommand_pos previous (tCommand_pos pos, tHist list); //returns the position of the item before the given
bool insertItem(struct tNode node, tHist *list); // inserts item with given contents
void deleteAtPosition(tCommand_pos pos, tHist *list); //deletes item from the list at a given position
tCommand getItem(tCommand_pos pos, tHist list); //returns the contents of an element at a given position

#endif //SHELL_HIST_H

