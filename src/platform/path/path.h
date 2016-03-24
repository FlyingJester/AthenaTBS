#pragma once
#include "../export.h"

/* Returned value must be freed by the caller. */
ATHENA_PLATFORM_EXPORT char *Athena_CanonizePathName(const char * const in);

/* If `in' is a directory name, then a copy of `in' is returned.
 * Otherwise, the directory that contains `in' is returned.
 * Returned value must be freed by the caller. 
 */
ATHENA_PLATFORM_EXPORT char *Athena_GetContainingDirectory(const char * const in);

/* Modifies `in' */
ATHENA_PLATFORM_EXPORT void Athena_GetContainingDirectoryStatic(char * in);
ATHENA_PLATFORM_EXPORT void Athena_GetContainingDirectoryStaticN(char * in, unsigned len);

ATHENA_PLATFORM_EXPORT unsigned Athena_IsDirectory(const char * const in);
ATHENA_PLATFORM_EXPORT unsigned Athena_IsFile(const char * const in);
