#include "../include/CalculateGeometry.h"

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
    BYTE *sectorsPerTrack)
{
    UINT64 cylindersTimesHeads;

    if (totalSectors > VHD_MAX_GEOMETRY)
        totalSectors = VHD_MAX_GEOMETRY;

    if (totalSectors >= (UINT64)65535 * 16 * 63)
    {
        *sectorsPerTrack = 255;
        *heads = 16;
        cylindersTimesHeads = totalSectors / *sectorsPerTrack;
    }
    else
    {
        *sectorsPerTrack = 17;
        cylindersTimesHeads = totalSectors / *sectorsPerTrack;

        *heads = (BYTE)((cylindersTimesHeads + 1023) / 1024);

        if (*heads < 4)
            *heads = 4;

        if (cylindersTimesHeads >= (UINT64)*heads * 1024 ||
            *heads > 16)
        {
            *sectorsPerTrack = 31;
            *heads = 16;
            cylindersTimesHeads = totalSectors / *sectorsPerTrack;
        }

        if (cylindersTimesHeads >= (UINT64)*heads * 1024)
        {
            *sectorsPerTrack = 63;
            *heads = 16;
            cylindersTimesHeads = totalSectors / *sectorsPerTrack;
        }
    }

    *cylinders = (USHORT)(cylindersTimesHeads / *heads);
}
