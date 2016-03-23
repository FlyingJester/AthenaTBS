#include "path.h"
#include <Windows.h>

unsigned Athena_IsDirectory(const char * const in) {
	const DWORD attrib = GetFileAttributes(in);
	return (attrib != INVALID_FILE_ATTRIBUTES) && (attrib & FILE_ATTRIBUTE_DIRECTORY);
}

unsigned Athena_IsFile(const char * const in) {
	const DWORD attrib = GetFileAttributes(in);
	return (attrib != INVALID_FILE_ATTRIBUTES) && (attrib & FILE_ATTRIBUTE_NORMAL);
}
