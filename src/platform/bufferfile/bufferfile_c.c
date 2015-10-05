#include "bufferfile.h"
#include <stdio.h>
#include <stdlib.h>

void *BufferFile(const char *file, int *size){
    if(!file || !size){
        return NULL;
    }
    else{
        FILE * const that = fopen(file, "r");
        unsigned to = 0, capacity = 0x100;
        char *buffer = NULL;
        
        if(!that){
            fputs("[BufferFile] Could not open file ", stderr);
            fputs(file, stderr);
            fputc('\n', stderr);
            return NULL;
        }

        /* Load the file. */
        do{
            capacity <<= 1;
            if(buffer)
                buffer = realloc(buffer, capacity);
            else
                buffer = malloc(capacity);
            to += fread(buffer+to, 1, capacity-to, that);
            buffer[to] = 0;
        }while(!feof(that));

        size[0] = to;
        fclose(that);
        return buffer;
    }
}

void FreeBufferFile(void *in, int X){
    if(!X) return;
    free(in);
}
