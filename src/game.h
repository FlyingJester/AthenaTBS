#pragma once
#include "field.h"

typedef unsigned (*athena_field_condition)(const struct Athena_Field *field, unsigned num_players);
unsigned Athena_ConquestCondition(const struct Athena_Field *field, unsigned num_players);

int Athena_Game(struct Athena_Field *field, unsigned num_players, athena_field_condition win_condition);
