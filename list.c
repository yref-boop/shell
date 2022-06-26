#include "list.h"
#include <string.h>

//FUNCTION IMPLEMENTATION:

bool isEmptyList (tList l) {
    return (l==NULL); //it is checked if the pointer ot the first element is null or not, if it is, the list is empty
}

void createEmptyList (tList *list){
    *list = NULL_TEXT; //condition: variable list (initial item), points to nothing
}

tPos first(tList list){
    return list; //l points to the first item
}

tPos next (tPos pos, tList list){
    return pos->next; //the next item is obtained
}

tPos last(tList list){

    //DESCRIPTION: looks for the position of the last element of the list
    //INPUTS: the list in which we search for the last user
    //OUTPUT: the position of the user that is at last position (tUserPos->next  NULL_TEXT   //PRECONDITIONS: the list must be previously initialised and it cannot be empty
    //POSCONDITIONS:

    tPos pos; //we create a variable position

    for (pos=list; pos->next != NULL_TEXT ; pos = pos->next); //we do a sequential search, from one, one towards the end if next isn't NULL_TEXT
    return pos; //this way, the item whose next is NULL_TEXTined (the one that doesn't have an item after it)
}

bool insertItem(struct tNode node, tList *list){

    tPos n_pos, p_pos; //needed position variables, new node position and the position that will be its previous
    n_pos = malloc (sizeof(struct tNode)); //check if there is enough memory to insert the item

    if (n_pos == NULL_TEXT) //if there is not enough space:
        return false; //the item can't be inserted
    else {
        //there is enough memory, n_pos = position of the new node
        strcpy(n_pos->data.text, node.data.text); //n_pos gets the given data

        if (node.mem.size >= 0) {
            n_pos->mem.address = node.mem.address;
            strcpy(n_pos->mem.file.text, node.mem.file.text);
            n_pos->mem.key = node.mem.key;
            n_pos->mem.size = node.mem.size;
            strcpy(n_pos->mem.date.text, node.mem.date.text);
        }

        n_pos->next = NULL_TEXT; //the next position to n_pos is set to NULL_TEXT  if (*list == NULL_TEXT) //list is empty

        if (*list == NULL_TEXT) //list is empty
            *list = n_pos; //n_pos is the first position
        else {
            p_pos = *list; //set the following position at the beginning of the list
            while (p_pos->next != NULL_TEXT)
                p_pos = p_pos->next;
            n_pos->next = p_pos->next; //set that the next of n_pos is the next of prev_pos
            p_pos->next = n_pos; //set n_pos to be the position following prev_pos
            //thus prev_pos is the position previous to n_pos
        }
        return true; //indeed, it is possible to insert the item
    }
}

void deleteAtPosition(tPos pos, tList *list){

    tPos p_pos;
    if ( pos == *list ) //given position is set to be the first element
        *list = pos->next; //list points to the next item of given position, thus ignoring it
    else if (pos->next == NULL_TEXT) { //given position is set to be the last element

        for ( p_pos=*list ; p_pos->next  != pos ; p_pos = p_pos->next); //q as l (first pos) advance though the list until the next to q is p
        //q is previous to p
        p_pos->next = NULL_TEXT; //point the next of q to NULL_TEXTaking it the last element in the list
    }
    else{ //intermediate position
        for ( p_pos=*list ; p_pos->next  != pos ; p_pos = p_pos->next);//prev_pos as first pos advance
        // though the list until the next to prev_pos is given position
        p_pos->next = pos->next; //once obtained previous position to given, we set the item that goes after
        // given position to go after previous position, thus skipping given pos
    }
    free(pos); // data in given position not needed anymore, thus freed
}

struct tNode getItem(tPos pos, tList list) {
    return *pos; //it gets the data of a given item at a given position
}