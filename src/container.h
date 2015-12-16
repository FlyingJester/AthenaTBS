#pragma once

void *Athena_AssureCapacity(void *data, unsigned element_size, unsigned size, unsigned *capacity);
void *Athena_AddOneCapacity(void *data, unsigned element_size, unsigned *size, unsigned *capacity);
void Athena_FreeContainer(void **that, unsigned size);
