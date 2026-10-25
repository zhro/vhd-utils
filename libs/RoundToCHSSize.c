
#define VHD_MAX_GEOMETRY ((UINT64)65535 * 16 * 255)

static BOOL RoundToCHSSize(UINT64 *sizeBytes)
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
