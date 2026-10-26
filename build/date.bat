@echo off

:: Formats and prints the current date, optionally stripping dashes when the -v
:: flag is provided.

for /f "tokens=2-4 delims=/ " %%a in ("%DATE%") do (
   set "month=%%a"
   set "day=%%b"
   set "year=%%c"
)

:: Prints the formatted date (the -v option was not used)
if not "%~1"=="-v" (
   echo %year%-%month%-%day%
   
   exit /b
)

:: Prints the formatted date with dashes removed
echo %year%%month%%day%
