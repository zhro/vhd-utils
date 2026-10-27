@echo off

set ERROR_STATUS=0

if /i "%1"=="-x86" (
    call :clean_x86
    exit /b %ERROR_STATUS%
)

if /i "%1"=="-x64" (
    call :clean_x64
    exit /b %ERROR_STATUS%
)

if not "%1"=="" (
   if /i not "%1"=="--help" (
      echo Error: Invalid option '%1'.
      exit /b 1
   )
   echo Usage: clean.bat [-x86 ^| -x64 ^| --help]
   exit /b 0
)

:: No options passed: invoke both subroutines and clean dist
call :clean_x86
call :clean_x64
call :safe_del "dist\*"
call :safe_rmdir "build\staging"
touch dist\.gitignore

exit /b %ERROR_STATUS%

:: ############################################################################
:: # Subroutines
:: ############################################################################

:clean_x86
   call :safe_rmdir "build\bin\x86"
   call :safe_rmdir "build\obj\x86"
exit /b

:clean_x64
   call :safe_rmdir "build\bin\x64"
   call :safe_rmdir "build\obj\x64"
exit /b

:safe_del
   if exist %1 (
      for %%F in (%1) do (
         del /f "%%F" /q 2>nul
         if exist "%%F" (
            echo Error: Failed to delete locked file: %%F
            set ERROR_STATUS=1
         )
      )
   )
exit /b

:safe_rmdir
   if exist %1 (
      rmdir /s /q %1
      if errorlevel 1 (
         echo Error: Failed to remove directory %1. It may be locked or in use.
         set ERROR_STATUS=1
      )
   )
exit /b
