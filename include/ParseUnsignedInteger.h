#ifndef __PARSENUMBER_H__
#define __PARSENUMBER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

BOOL __fastcall ParseUnsignedInteger( PCTSTR pszIn, PUINT64 puiOut );

#ifdef __cplusplus
}
#endif

#endif
