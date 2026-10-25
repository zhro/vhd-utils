#pragma comment(linker, "/version:1.0") // MUST be in the form of major.minor

#define INITGUID

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <virtdisk.h>
#include "libs\StringIntrinsics.h"
#include "libs\ParseNumber.h"

__forceinline BOOLEAN IsFlag( PCTSTR pszArg );
__forceinline BOOLEAN CheckFlagI( PCTSTR pszArg, TCHAR ch );

#define VHD_MAX_GEOMETRY ((UINT64)65535 * 16 * 255)
#define VHD_MINIMUM_SIZE 3168256ULL

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

static DWORD UpdateVhdFooterAt(
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

static DWORD SetVhdMaximumGeometry(PTSTR pszPath)
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

UINT __fastcall EntryMain( UINT argc, PTSTR *argv )
{
   PTSTR pszTargetPath = NULL;
   PTSTR pszTargetSize = NULL;
   PTSTR pszSourcePath = NULL;
   UINT64 cbTargetSize = 0;
   BOOLEAN fDynamic = FALSE;
   BOOLEAN fForceSize = FALSE;
   BOOLEAN fDifferencing = FALSE;
   BOOLEAN fShowUsage = FALSE;
   BOOLEAN fCompatibility = FALSE;
   
   // Parse command line parameters

   UINT i;

   for (i = 1; i < argc; ++i)
   {
      if (!IsFlag(argv[i]))
      {
         if (pszTargetPath == NULL)
            pszTargetPath = argv[i];
         else if (pszTargetSize == NULL)
            pszTargetSize = argv[i];
         else if (pszSourcePath == NULL)
            pszSourcePath = argv[i];
         else
            fShowUsage = TRUE;
      }
      else if (!fDynamic && CheckFlagI(argv[i], TEXT('d')))
         fDynamic = TRUE;
      else if (!fDifferencing && CheckFlagI(argv[i], TEXT('f')))
         fDifferencing = TRUE;
      else if (!fForceSize && CheckFlagI(argv[i], TEXT('s')))
         fForceSize = TRUE;
      else if (!fCompatibility && CheckFlagI(argv[i], TEXT('c')))
         fCompatibility = TRUE;
      else
         fShowUsage = TRUE;
   }
   
   if (fForceSize && fCompatibility)
   {
       _ftprintf(stderr,
           TEXT("Warning: -s and -c are mutually exclusive; -s takes precedence.\n"));
       fCompatibility = FALSE;
   }
   
   // ForceSize and Compatibility are mutually exclusive, with ForceSize
   // taking precedence
   if (fForceSize)
       fCompatibility = FALSE;
   
   if (fDifferencing)
   {
      // It is possible for the -f flag to be specified at the end, in which
      // case, the meaning of the second non-flag parameter can only be
      // determined after a complete pass of the parameters.

      if (pszTargetSize && !pszSourcePath)
         pszSourcePath = pszTargetSize;
      else
         fShowUsage = TRUE;
   }
   
   // Parse the target size

   else if (!pszTargetSize)
      fShowUsage = TRUE;
   else
   {
      PTSTR pszSize = _tcsdup(pszTargetSize);
      UINT64 cbMultiplier = 1;
      size_t cchSize;

      if (pszSize == NULL)
         fShowUsage = TRUE;
      else
      {
         cchSize = _tcslen(pszSize);

         if (cchSize > 0)
         {
            switch (_totlower(pszSize[cchSize - 1]))
            {
            case TEXT('b'):
               cbMultiplier = 1;
               pszSize[cchSize - 1] = 0;
               break;

            case TEXT('k'):
               cbMultiplier = (UINT64)1024;
               pszSize[cchSize - 1] = 0;
               break;

            case TEXT('m'):
               cbMultiplier = (UINT64)1024 * 1024;
               pszSize[cchSize - 1] = 0;
               break;

            case TEXT('g'):
               cbMultiplier = (UINT64)1024 * 1024 * 1024;
               pszSize[cchSize - 1] = 0;
               break;

            case TEXT('t'):
               cbMultiplier =
                  (UINT64)1024 * 1024 * 1024 * 1024;
               pszSize[cchSize - 1] = 0;
               break;
            }
         }

         if (!ParseUnsignedInteger(pszSize, &cbTargetSize) ||
            cbTargetSize > ((UINT64)-1) / cbMultiplier)
         {
            fShowUsage = TRUE;
         }
         else
         {
            cbTargetSize *= cbMultiplier;
         }

         free(pszSize);
         
         if (cbTargetSize != 0 && cbTargetSize < VHD_MINIMUM_SIZE)
         {
             _ftprintf(stderr,
                 TEXT("Error: The minimum supported image size is 3168256 bytes (3 megabytes).\n"));
             return ERROR_INVALID_PARAMETER;
         }
      }
   }

   // Act according to the command line parameters

   if (fShowUsage)
   {
      _tprintf(TEXT("Usage: makevhd [-d] [-c] [-s] vhd size [source]\n\n"));
      _tprintf(TEXT("  -d      Use dynamic allocation for the virtual disk.\n")
               TEXT("  -c      Use Windows-default CHS geometry behavior.\n")
               TEXT("  -s      Preserve the requested size and use maximum CHS geometry.\n")
               TEXT("  vhd     Path of the VHD file to be created.\n")
               TEXT("  size    Maximum size of the VHD; must be a multiple of 512.\n")
               TEXT("          Suffixes: b=bytes (default), k=KiB, m=MiB, g=GiB, t=TiB.\n")
               TEXT("          If set to 0, the size of the source disk, if available, will be used.\n")
               TEXT("  source  Path of an optional source whose data will populate the new VHD.\n\n"));

      _tprintf(TEXT("Usage: makevhd -f vhd parent\n\n"));
      _tprintf(TEXT("  -f      Create a differencing VHD.\n")
               TEXT("  vhd     Path of the differencing VHD file to be created.\n")
               TEXT("  parent  Path of an existing disk to be associated as the parent.\n"));

      return ~1;
   }
   else
   {
      VIRTUAL_STORAGE_TYPE vst;
      CREATE_VIRTUAL_DISK_PARAMETERS cvdp;

      HANDLE hVhd;
      DWORD dwResult;
      DWORD cchMessage;
      PTSTR pszErrorMessage = NULL;

      PTSTR pszTargetExt = _tcsrchr(pszTargetPath, TEXT('.'));

      vst.DeviceId = (pszTargetExt && lstrcmpi(pszTargetExt + 1, TEXT("vhdx")) == 0) ?
         VIRTUAL_STORAGE_TYPE_DEVICE_VHDX :
         VIRTUAL_STORAGE_TYPE_DEVICE_VHD;
      vst.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_MICROSOFT;
      
      if (!fForceSize && !fCompatibility &&
          !RoundToCHSSize(&cbTargetSize))
      {
          _ftprintf(stderr,
              TEXT("Error: Invalid or unrepresentable VHD size.\n"));
          return ERROR_INVALID_PARAMETER;
      }
      
      ZeroMemory(&cvdp, sizeof(cvdp));
      cvdp.Version = CREATE_VIRTUAL_DISK_VERSION_1;
      cvdp.Version1.MaximumSize = cbTargetSize;
      cvdp.Version1.SectorSizeInBytes = CREATE_VIRTUAL_DISK_PARAMETERS_DEFAULT_SECTOR_SIZE;

      if (fDifferencing)
         cvdp.Version1.ParentPath = pszSourcePath;
      else
         cvdp.Version1.SourcePath = pszSourcePath;

      dwResult = CreateVirtualDisk(
         &vst,
         pszTargetPath,
         VIRTUAL_DISK_ACCESS_CREATE,
         NULL,
         (fDynamic || fDifferencing) ? 0 : CREATE_VIRTUAL_DISK_FLAG_FULL_PHYSICAL_ALLOCATION,
         0,
         &cvdp,
         NULL,
         &hVhd
      );
      
      if (cbTargetSize != 0 && (cbTargetSize % 512) != 0)
      {
          if (dwResult == ERROR_INVALID_PARAMETER)
          {
              _ftprintf(stderr,
                  TEXT("Error: The requested size is not a multiple of 512 bytes.\n"));
              
              return dwResult;
          }
          else
          {
              _ftprintf(stderr,
                  TEXT("Warning: The requested size is not a multiple of 512 bytes.\n"));
          }
      }
      
      if (dwResult != ERROR_SUCCESS)
      {
         cchMessage = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dwResult,
            0,
            (PTSTR)&pszErrorMessage,
            0,
            NULL
         );
         
         if (cchMessage)
            _ftprintf(stderr, TEXT("%ls"), pszErrorMessage);
         else
            _ftprintf(stderr, TEXT("Code %08X\n"), dwResult);
      
         LocalFree(pszErrorMessage);
         
         return dwResult;
      }
      
      CloseHandle(hVhd);
      
      if (fForceSize)
         if (vst.DeviceId != VIRTUAL_STORAGE_TYPE_DEVICE_VHD)
            dwResult = ERROR_NOT_SUPPORTED;
         else
            dwResult = SetVhdMaximumGeometry(pszTargetPath);
      
      return dwResult;
   }
}

__forceinline BOOLEAN IsFlag( PCTSTR pszArg )
{
   return(
      (pszArg[0] | 0x02) == TEXT('/') &&
      (pszArg[1]       ) != 0 &&
      (pszArg[2]       ) == 0
   );
}

__forceinline BOOLEAN CheckFlagI( PCTSTR pszArg, TCHAR ch )
{
   return(
      (pszArg[1] | 0x20) == ch
   );
}
