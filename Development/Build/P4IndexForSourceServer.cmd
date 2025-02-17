@echo off
@REM ----------------------------------------
@REM Now just a stub to call SSIndex.cmd
@REM ----------------------------------------
set SRCSRV_SOURCE=C:\Build_UE3\UnrealEngine3
set SRCSRV_SYMBOLS=C:\Build_UE3\UnrealEngine3\Binaries
@call "%~dp0SSIndex.cmd" -SYSTEM=P4 %*
