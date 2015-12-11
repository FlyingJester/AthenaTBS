#pragma once

struct Athena_Resources{
    int cash, metal, food;
};

void Athena_AddResources(struct Athena_Resources *to, const struct Athena_Resources *from);
void Athena_SubResources(struct Athena_Resources *to, const struct Athena_Resources *from);
