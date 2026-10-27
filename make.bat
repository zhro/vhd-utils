@echo off
setlocal

:: Check if any argument passed is --help
for %%a in (%*) do (
   if /i "%%~a"=="--help" (
      echo Usage: %~nx0 [CPPVER=version] [ARCH=architecture] [targets...]
      echo.
      echo Options:
      echo   CPPVER=version    Visual Studio toolset version (e.g., 110, 120, 140, 150^)
      echo   ARCH=architecture Target architecture (x86 or x64^)
      echo   --help            Display this help message
      echo.
      exit /b 0
   )
)

if "%VISUALSTUDIOVERSION%"=="" (
   echo Error: Visual Studio build environment not detected. Please open the 
   echo   appropriate Developer Command Prompt for Visual Studio (e.g., VS2015 x64
   echo   Native Tools Command Prompt or VS2015 x86 Native Tools Command Prompt^) to
   echo   ensure that all required environment variables are set.
   
   call :exit_with_pause
   exit /b 1
)

nmake.exe /nologo %*
exit /b

:: ############################################################################
:: # Subroutines
:: ############################################################################

REM Check if launched via double-click (explorer) vs active terminal
:exit_with_pause
echo %cmdcmdline% | find /i "/c" >nul
if not errorlevel 1 (
   echo.
   pause
)

exit /b 0
