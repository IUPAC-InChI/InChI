#ifndef _FUZZER_H_
#define _FUZZER_H_


#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
/* Windows */
#include <windows.h>
#define HANDLE_INSTANCE HINSTANCE
#define GET_DLSYM GetProcAddress
#define LIBINCHINAME "libinchi.dll"
#else
/* Linux */
#include <dlfcn.h>
#define HANDLE_INSTANCE void
#define GET_DLSYM dlsym
#define LIBINCHINAME "libinchi.so.1"
#endif


#ifdef __cplusplus
}

int load_inchi_library(void);
void unload_inchi_library(void);


#endif

#endif    /* _FUZZER_H_ */