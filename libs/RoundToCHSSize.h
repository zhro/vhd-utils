#ifndef __ROUNDTOCHSSIZE_H__
#define __ROUNDTOCHSSIZE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

#define VHD_MAX_GEOMETRY ((UINT64)65535 * 16 * 255)

/*
 * Round the requested virtual disk size up to the smallest size that can
 * be represented by the CHS geometry calculated by CalculateGeometry().
 *
 * The size must be a multiple of 512 bytes. If the requested size is zero,
 * it is left unchanged. Otherwise, the sector count is incremented until
 * the calculated CHS geometry represents at least the requested capacity.
 * The resulting size is updated to match the capacity represented by that
 * geometry.
 *
 * Parameters:
 *   sizeBytes  Pointer to the requested disk size in bytes; receives the
 *              adjusted size on success.
 *
 * Returns:
 *   TRUE  if the size is valid and successfully adjusted.
 *   FALSE if the size is not sector-aligned or cannot be represented
 *         within the supported CHS limits.
 */
BOOL __fastcall RoundToCHSSize(UINT64 *sizeBytes);

#ifdef __cplusplus
}
#endif

#endif
