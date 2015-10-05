#pragma once
struct Turbo_Value;

/* Returns NULL if value was not found. */
struct Turbo_Value *Turbo_Helper_GetObjectElement(struct Turbo_Value *from, const char *name);
const struct Turbo_Value *Turbo_Helper_GetConstObjectElement(const struct Turbo_Value *from, const char *name);
