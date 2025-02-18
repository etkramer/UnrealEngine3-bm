@echo off

set GAMENAME=%1
set CONFIG=%2

set XEDK=
set DXSDK_DIR="%CD%\Development\External\dxsdk_aug2007"

call "%VS80COMNTOOLS%\..\..\VC\vcvarsall.bat"
echo:

cd Development/Src
"../Intermediate/UnrealBuildTool/Release/UnrealBuildTool.exe" %GAMENAME% Win32 %CONFIG% -output ../../Binaries/%CONFIG%-%GAMENAME%.exe