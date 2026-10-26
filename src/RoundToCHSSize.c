#include "../include/RoundToCHSSize.h"

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
BOOL __fastcall RoundToCHSSize(UINT64 *sizeBytes)
{
    UINT64 requestedSectors;
    UINT64 candidateSectors;
    UINT64 geometrySectors;
    USHORT cylinders = 0;
    BYTE heads = 0;
    BYTE sectorsPerTrack = 0;

    if (*sizeBytes == 0)
        return TRUE;

    if ((*sizeBytes % 512) != 0)
        return FALSE;

    requestedSectors = *sizeBytes / 512;

    /*
     * QEMU uses the requested size directly when the maximum
     * representable CHS geometry is reached.
     */
    if (requestedSectors >= VHD_MAX_GEOMETRY)
        return TRUE;

    candidateSectors = requestedSectors;

    for (;;)
    {
        CalculateGeometry(
            candidateSectors,
            &cylinders,
            &heads,
            &sectorsPerTrack);

        geometrySectors =
            (UINT64)cylinders * heads * sectorsPerTrack;

        if (geometrySectors >= requestedSectors)
            break;

        if (candidateSectors == VHD_MAX_GEOMETRY)
            return FALSE;

        ++candidateSectors;
    }

    *sizeBytes = geometrySectors * 512;
    return TRUE;
}
