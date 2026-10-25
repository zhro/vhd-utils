
static void CalculateGeometry(
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
