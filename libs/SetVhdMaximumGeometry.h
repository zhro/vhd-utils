#ifndef __SETVHDMAXIMUMGEOMETRY_H__
#define __SETVHDMAXIMUMGEOMETRY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

/*
 * Set the CHS geometry in a VHD file's footer to the maximum representable
 * values (65535 cylinders, 16 heads, and 255 sectors per track), preserving
 * the virtual disk's actual capacity.
 *
 * Recalculates the footer checksum after modifying the geometry fields.
 * For dynamic and differencing VHDs, updates both the leading and trailing
 * footers when present.
 *
 * Parameters:
 *   pszVhdPath  Path to the VHD file to modify.
 *
 * Returns:
 *   ERROR_SUCCESS if the geometry is updated successfully; otherwise,
 *   a Windows error code describing the failure.
 */
DWORD __fastcall SetVhdMaximumGeometry(PTSTR pszPath);

#ifdef __cplusplus
}
#endif

#endif
