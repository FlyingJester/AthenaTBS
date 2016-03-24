#ifndef ATHENA_PLATFORM_EXPORT

#ifdef __cplusplus
#define ATHENA_PLATFORM_CDECL extern "C"
#else 
#define ATHENA_PLATFORM_CDECL
#endif

#ifdef _WIN32
#ifdef ATHENAPLATFORM_EXPORTS
#define ATHENA_PLATFORM_EXPORT ATHENA_PLATFORM_CDECL __declspec(dllexport)
#else
#define ATHENA_PLATFORM_EXPORT ATHENA_PLATFORM_CDECL __declspec(dllimport)
#endif
#else
#define ATHENA_PLATFORM_EXPORT ATHENA_PLATFORM_CDECL
#endif

#endif