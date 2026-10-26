#ifndef __CALCULATEGEOMETRY_H__
#define __CALCULATEGEOMETRY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

#define VHD_MAX_GEOMETRY ((UINT64)65535 * 16 * 255)

void __fastcall CalculateGeometry(
    UINT64 totalSectors,
    USHORT *cylinders,
    BYTE *heads,
    BYTE *sectorsPerTrack);

#ifdef __cplusplus
}
#endif

#endif
