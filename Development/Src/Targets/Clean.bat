@echo off
REM %1 is the game name
REM %2 is the platform name
REM %3 is the configuration name

IF EXIST ..\Intermediate\%2\%3\%1 (
     del ..\Intermediate\%2\%3\%1\*.* /s /q >NUL 2>&1
)
