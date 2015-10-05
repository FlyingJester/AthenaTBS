#pragma once
#include "field.h"

struct Athena_AI{
    unsigned owner;
    
};

int Athena_TickAI(struct Athena_AI *ai, struct Athena_Field *field);
