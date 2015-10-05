#include "private_window.h"

void *Athena_Private_CreateHandle(){
    return 1;
}

int Athena_Private_DestroyHandle(void *x){
    if(x==1)
        return 0;
    return -1;
}

int Athena_Private_ShowWindow(void * x){
    if(!x)
        return -1;
    return 0;
}

int Athena_Private_HideWindow(void *x){
    if(!x)
        return -1;
    return 0;
}
