#ifndef DYN_ARRAY
#define DYN_ARRAY

#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

struct DynArr{
    int size;
    int capacity;
    pid_t *myArray;
};

void createArray(struct DynArr *, int);
void arrayAdd(struct DynArr *, pid_t);
void arrayDelete(struct DynArr *);
int arrayContains(struct DynArr *, pid_t);
int arrayIndexOf(struct DynArr *, pid_t);
void arrayRemoveAt(struct DynArr *, int);

#endif
