REM %1 is the game name

call C:\Build_UE3\UnrealEngine3\binaries\xenon\setXbox360EnvVars.bat
call C:\Build_UE3\UnrealEngine3\binaries\PS3\setPS3EnvVars.bat

REM xenon implies PC
call C:\Build_UE3\UnrealEngine3\binaries\%1.com precompileshaders platform=pc -refcache -unattended -skipmaps -ALLOW_PARALLEL_PRECOMPILESHADERS LOG=Launch.log
call C:\Build_UE3\UnrealEngine3\binaries\%1.com precompileshaders platform=xenon -refcache -unattended -skipmaps -ALLOW_PARALLEL_PRECOMPILESHADERS LOG=Launch.log
call C:\Build_UE3\UnrealEngine3\binaries\%1.com precompileshaders platform=ps3 -refcache -unattended -skipmaps -ALLOW_PARALLEL_PRECOMPILESHADERS LOG=Launch.log
call C:\Build_UE3\UnrealEngine3\binaries\%1.com precompileshaders platform=pc_sm2 -refcache -unattended -skipmaps -ALLOW_PARALLEL_PRECOMPILESHADERS LOG=Launch.log


