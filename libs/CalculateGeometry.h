#ifndef __CALCULATEGEOMETRY_H__
#define __CALCULATEGEOMETRY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

/*
 * Calculate the CHS (Cylinder-Head-Sector) geometry for a virtual disk
 * based on its total number of sectors, following the geometry conventions
 * used by QEMU for VHD images.
 *
 * The geometry is selected according to the disk capacity, using 17, 31,
 * 63, or 255 sectors per track as appropriate, with up to 16 heads and
 * 65535 cylinders. If the disk exceeds the maximum representable CHS
 * capacity, the sector count is capped for geometry calculation.
 *
 * Parameters:
 *   totalSectors     Total number of sectors in the virtual disk.
 *   cylinders        Receives the calculated number of cylinders.
 *   heads            Receives the calculated number of heads.
 *   sectorsPerTrack  Receives the calculated sectors per track.
 */
void __fastcall CalculateGeometry(
    UINT64 totalSectors,
    USHORT *cylinders,
    BYTE *heads,
    BYTE *sectorsPerTrack);

#ifdef __cplusplus
}
#endif

#endif
