@echo off
REM %1 is the game name
REM %2 is the platform name
REM %3 is the configuration name
REM %4 is the output path.

if exist ..\Intermediate\%2\%3\%1 (
     del ..\Intermediate\%2\%3\%1\*.* /s /q >NUL 2>&1
)
if exist ..\Intermediate\%2\%3\%1*.strelf (
     del ..\Intermediate\%2\%3\%1*.strelf /s /q >NUL 2>&1
)

if exist %4 (
     del %4 /q >NUL 2>&1
)
