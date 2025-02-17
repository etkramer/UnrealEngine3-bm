MSBuild UnrealBuildTool/UnrealBuildTool.csproj /target:Build /property:Configuration=Release

p4 edit ..\..\Binaries\ExampleGame.exe

..\Intermediate\UnrealBuildTool\Release\UnrealBuildTool.exe Win32 Release ExampleGame -output ..\..\Binaries\ExampleGame.exe -noxge

