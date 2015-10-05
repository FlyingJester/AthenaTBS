#pragma once

/* Returned value must be freed by the caller. */
char *Athena_CanonizePathName(const char * const in);

/* If `in' is a directory name, then a copy of `in' is returned.
 * Otherwise, the directory that contains `in' is returned.
 * Returned value must be freed by the caller. 
 */
char *Athena_GetContainingDirectory(const char * const in);

/* Modifies `in' */
void Athena_GetContainingDirectoryStatic(char * in);
void Athena_GetContainingDirectoryStaticN(char * in, unsigned len);

unsigned Athena_IsDirectory(const char * const in);
unsigned Athena_IsFile(const char * const in);
