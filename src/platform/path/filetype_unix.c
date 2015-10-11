#include "path.h"
#include <sys/stat.h>
#include <fcntl.h>

unsigned Athena_IsDirectory(const char * const in){
    struct stat l_stat;
    stat(in,  &l_stat);

    return S_ISDIR(l_stat.st_mode);
}

unsigned Athena_IsFile(const char * const in){
    struct stat l_stat;
    stat(in,  &l_stat);

    return !(S_ISDIR(l_stat.st_mode) || S_ISBLK(l_stat.st_mode));
}
