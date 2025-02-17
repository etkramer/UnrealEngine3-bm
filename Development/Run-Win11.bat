@echo off

set GAMENAME=%1
set CONFIG=%2

:: TODO - This should "just work". Instead, we had to copy everything in here to Binaries. Find out why?
set PATH=%PATH%;"I:\UnrealEngine3-gears2\Development\External\Developer Runtime\x86\"

cd ../Binaries
%CONFIG%-%GAMENAME%.exe editor -log