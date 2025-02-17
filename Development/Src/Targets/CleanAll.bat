@echo off

if exist ..\Intermediate\Win32 (
     del ..\Intermediate\Win32 /s /q >NUL 2>&1
)
if exist ..\Intermediate\Xbox360 (
     del ..\Intermediate\Xbox360 /s /q >NUL 2>&1
)
