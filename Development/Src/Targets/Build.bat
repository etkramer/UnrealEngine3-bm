@echo off

call "%VS80COMNTOOLS%\vsvars32.bat" >NUL 2>&1

if exist ..\Intermediate\UnrealBuildTool\Release\UnrealBuildTool.exe (
         ..\Intermediate\UnrealBuildTool\Release\UnrealBuildTool.exe %* -DEPLOY
)
