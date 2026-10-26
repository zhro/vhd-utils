@echo off

for /f "tokens=2-4 delims=/ " %%a in ("%DATE%") do (
    set "month=%%a"
    set "day=%%b"
    set "year=%%c"
)
echo %year%-%month%-%day%
