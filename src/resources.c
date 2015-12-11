#include "resources.h"

void Athena_AddResources(struct Athena_Resources *to, const struct Athena_Resources *from){
    to->cash += from->cash;
    to->metal += from->metal;
    to->food += from->food;
}

void Athena_SubResources(struct Athena_Resources *to, const struct Athena_Resources *from){
    to->cash -= from->cash;
    to->metal -= from->metal;
    to->food -= from->food;
}
