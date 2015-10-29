#pragma once
struct Turbo_Value;

/* Returns NULL if value was not found. */
struct Turbo_Value *Turbo_Helper_GetObjectElement(struct Turbo_Value *from, const char *name);
const struct Turbo_Value *Turbo_Helper_GetConstObjectElement(const struct Turbo_Value *from, const char *name);

/* Returns 0 on failure (so NOT like strcmp or memcmp) */
unsigned Turbo_Helper_CompareString(const struct Turbo_Value *str_value, const char *with, unsigned len);
#define Turbo_Helper_CompareStringConstant(STR_VALUE, THAT)\
    Turbo_Helper_CompareString(STR_VALUE, THAT, (sizeof THAT)-1)
