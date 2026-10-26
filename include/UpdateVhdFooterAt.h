#ifndef __UPDATEVHDFOOTERAT_H__
#define __UPDATEVHDFOOTERAT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

DWORD __fastcall UpdateVhdFooterAt(
    HANDLE hFile,
    LARGE_INTEGER offset);

#ifdef __cplusplus
}
#endif

#endif
