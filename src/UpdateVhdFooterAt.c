#include "../include/UpdateVhdFooterAt.h"

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
    LARGE_INTEGER offset)
{
    BYTE footer[512];
    DWORD cbRead;
    DWORD cbWritten;
    DWORD i;
    DWORD sum = 0;
    DWORD checksum;
    LARGE_INTEGER position;

    if (!SetFilePointerEx(hFile, offset, &position, FILE_BEGIN))
        return GetLastError();

    if (!ReadFile(hFile, footer, sizeof(footer), &cbRead, NULL))
        return GetLastError();

    if (cbRead != sizeof(footer))
        return ERROR_HANDLE_EOF;

    if (memcmp(footer, "conectix", 8) != 0)
        return ERROR_BAD_FORMAT;

    /* Maximum VHD CHS geometry: 65535 / 16 / 255. */
    footer[56] = 0xFF;
    footer[57] = 0xFF;
    footer[58] = 16;
    footer[59] = 255;

    /* Clear the checksum field before calculating it. */
    memset(footer + 64, 0, 4);

    for (i = 0; i < sizeof(footer); ++i)
        sum += footer[i];

    checksum = ~sum;

    /* Store the checksum in big-endian byte order. */
    footer[64] = (BYTE)(checksum >> 24);
    footer[65] = (BYTE)(checksum >> 16);
    footer[66] = (BYTE)(checksum >> 8);
    footer[67] = (BYTE)checksum;

    if (!SetFilePointerEx(hFile, offset, &position, FILE_BEGIN))
        return GetLastError();

    if (!WriteFile(hFile, footer, sizeof(footer), &cbWritten, NULL))
        return GetLastError();

    if (cbWritten != sizeof(footer))
        return ERROR_WRITE_FAULT;

    return ERROR_SUCCESS;
}
