@echo off

set GAMENAME=%1
set CONFIG=%2

set XEDK=
set DXSDK_DIR="I:\UnrealEngine3-gears2\Development\External\dxsdk_aug2007"

call "C:\Program Files (x86)\Microsoft Visual Studio 8\VC\vcvarsall.bat"

cd Src
"../Intermediate/UnrealBuildTool/Release/UnrealBuildTool.exe" %GAMENAME% Win32 %CONFIG% -output ../../Binaries/%CONFIG%-%GAMENAME%.exe