#include "../include/SetVhdMaximumGeometry.h"
#include "../include/UpdateVhdFooterAt.h"

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
DWORD __fastcall SetVhdMaximumGeometry(PTSTR pszPath)
{
    HANDLE hFile;
    LARGE_INTEGER fileSize;
    LARGE_INTEGER offset;
    LARGE_INTEGER position;
    BYTE cookie[8];
    DWORD cbRead;
    DWORD dwResult;

    hFile = CreateFile(
        pszPath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return GetLastError();

    if (!GetFileSizeEx(hFile, &fileSize))
    {
        dwResult = GetLastError();
        CloseHandle(hFile);
        return dwResult;
    }

    if (fileSize.QuadPart < 512)
    {
        CloseHandle(hFile);
        return ERROR_BAD_FORMAT;
    }

    /* The trailing footer exists for fixed, dynamic and differencing VHDs. */
    offset.QuadPart = fileSize.QuadPart - 512;

    dwResult = UpdateVhdFooterAt(hFile, offset);

    if (dwResult != ERROR_SUCCESS)
    {
        CloseHandle(hFile);
        return dwResult;
    }

    /*
     * Dynamic and differencing VHDs also have a footer at offset zero.
     * Patch it only if the cookie identifies a VHD footer.
     */
    offset.QuadPart = 0;

    if (!SetFilePointerEx(hFile, offset, &position, FILE_BEGIN))
    {
        dwResult = GetLastError();
        CloseHandle(hFile);
        return dwResult;
    }

    if (!ReadFile(hFile, cookie, sizeof(cookie), &cbRead, NULL))
    {
        dwResult = GetLastError();
        CloseHandle(hFile);
        return dwResult;
    }

    if (cbRead == sizeof(cookie) &&
        memcmp(cookie, "conectix", sizeof(cookie)) == 0)
    {
        dwResult = UpdateVhdFooterAt(hFile, offset);

        if (dwResult != ERROR_SUCCESS)
        {
            CloseHandle(hFile);
            return dwResult;
        }
    }

    CloseHandle(hFile);
    return ERROR_SUCCESS;
}
