@echo off

setlocal enabledelayedexpansion

:: Print the version number from the VERSION file, or check against  README.md
:: and display a warning if they don't match when using -w.

set PROJECT_ROOT=%~dp0..
cd "%PROJECT_ROOT%"

:: VERSION file is missing
if not exist "VERSION" (
   echo Error: VERSION file not found.
   exit /b 1
)

set /p VERSION=<"VERSION"

:: Print the version number and exit
if /i not "%~1"=="-w" (
   echo %VERSION%
   exit /b
)

:: README.md file is missing
if not exist README.md (
   echo Error: README.md not found.
   exit /b 1
)

:: Test the value read from the VERSION file against the version number parsed
:: from README.md
for /f "tokens=2 delims=*" %%R in ('findstr /b "**Version:**" README.md') do (
   set "README_VER=%%R"
)
set "README_VER=%README_VER:~1%"

:: Display a warning if they don't match
if not "%VERSION%"=="%README_VER%" (
   echo WARNING: VERSION file ^(%VERSION%^) does not match README.md version ^(%README_VER%^).
)
