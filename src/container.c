#include "container.h"
#include <stdlib.h>
#include <string.h>

void *Athena_AssureCapacity(void *data, unsigned element_size, unsigned size, unsigned *capacity){
    unsigned c = *capacity;
    if(size > c){
        if(c<0x100)
            c += 0x100;
        else
            c <<= 1;
        
        if(!data)
            data = malloc(element_size * c);
        else
            data = realloc(data, element_size * c);

        capacity[0] = c;
    }
    
    return data;
}

void *Athena_AddOneCapacity(void *data, unsigned element_size, unsigned *size, unsigned *capacity){
    size[0]++;
    return Athena_AssureCapacity(data, element_size, *size, capacity);
}

void Athena_FreeContainer(void **that, unsigned size){
    if(size){
        free(that[0]);
        Athena_FreeContainer(that+1, size-1);
    }
}
