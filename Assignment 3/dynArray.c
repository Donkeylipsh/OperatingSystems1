#include "dynArray.h"
//#include <stdio.h>

void createArray(struct DynArr *dyn, int capacity){
    dyn->size = 0;
    dyn->capacity = capacity;
    dyn->myArray = malloc(sizeof(pid_t) * capacity);
}

void arrayAdd(struct DynArr *dyn, pid_t somePid){
    if(dyn->size < dyn->capacity){
	dyn->myArray[dyn->size] = somePid;
	dyn->size++;
    }else{
	pid_t *newDyn = malloc(sizeof(pid_t) * dyn->capacity * 2);
	int i;
	for(i = 0; i < dyn->size; i++){
	    newDyn[i] = dyn->myArray[i];
	}
	newDyn[dyn->size] = somePid;
	free(dyn->myArray);
	dyn->myArray = newDyn;
    }
}

void arrayDelete(struct DynArr *dyn){
    //printf("Freeing myArray\n");
    free(dyn->myArray);
}

int arrayContains(struct DynArr *dyn, pid_t somePid){
    int found = 0;
    int i;
    for(i = 0; i < dyn->size; i++){
	if(dyn->myArray[i] == somePid){
	    found == 1;
	}
    }

    return found;
}

int arrayIndexOf(struct DynArr *dyn, pid_t somePid){
    int index = -1;
    if(arrayContains(dyn, somePid)){
	index = 0;
	while(dyn->myArray[index] != somePid){
	    index++;
	}
    }
    return index;
}

void arrayRemoveAt(struct DynArr *dyn, int index){
    for(index; index < dyn->size; index++){
	dyn->myArray[index] = dyn->myArray[index + 1];
    }
    dyn->size--;
}
