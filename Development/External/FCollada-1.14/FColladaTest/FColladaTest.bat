@echo off

REM FS - Batch file used to run FColladaTest
REM Used by the BuildMachine (CruiseControl.NET)

call .\FColladaTest.exe -v 0 %*
