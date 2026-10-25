#ifndef __UPDATEVHDFOOTERAT_H__
#define __UPDATEVHDFOOTERAT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

/*
 * Update the CHS geometry and checksum of a VHD footer at the specified
 * file offset.
 *
 * Validates the VHD footer cookie, sets the geometry to the maximum
 * representable values (65535 cylinders, 16 heads, and 255 sectors per
 * track), recalculates the footer checksum, and writes the modified footer
 * back to the file.
 *
 * Parameters:
 *   hFile   Handle to the VHD file, opened with write access.
 *   offset  File offset of the VHD footer to update.
 *
 * Returns:
 *   ERROR_SUCCESS if the footer is updated successfully; otherwise,
 *   a Windows error code describing the failure.
 */
DWORD __fastcall UpdateVhdFooterAt(
    HANDLE hFile,
    LARGE_INTEGER offset);

#ifdef __cplusplus
}
#endif

#endif
