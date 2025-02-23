@echo off

set GAMENAME=%1
set CONFIG=%2

set XEDK=
set DXSDK_DIR="%CD%\External\dxsdk_aug2007"

call "%VS80COMNTOOLS%\..\..\VC\vcvarsall.bat"
echo:

cd Src

:: Build UBT
msbuild UnrealBuildTool/UnrealBuildTool.csproj /p:Configuration=Release /p:Platform=AnyCPU /nologo /verbosity:q

:: Run UBT (build game)
"../Intermediate/UnrealBuildTool/Release/UnrealBuildTool.exe" %GAMENAME% Win32 %CONFIG% -output ../../Binaries/%CONFIG%-%GAMENAME%.exe