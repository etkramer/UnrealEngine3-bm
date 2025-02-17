set LABEL_TO_SYNC_TO=latestUnrealEngine3



IF EXIST "\\Build-Server-01\BuildFlags\buildUE3PCS_ALL.txt" goto END
IF EXIST "\\Build-Server-01\BuildFlags\buildUE3PCS_Full.txt" goto END

IF EXIST "\\Build-Server-01\BuildFlags\OnlyUseQABuildSemaphore.txt" set LABEL_TO_SYNC_TO=currentQABuildInTesting

p4 sync //depot/UnrealEngine3/...@%LABEL_TO_SYNC_TO%

p4 sync //depot/UnrealEngine3/ExampleGame/Content/...
p4 sync //depot/UnrealEngine3/GearGame/Content/...
p4 sync //depot/UnrealEngine3/UTGame/Content/...
p4 sync //depot/UnrealEngine3/WarGame/Content/...

TriggerBuild_PreCompileShaders_All.bat


:END
REM do nothing as we are alreadying running the process
