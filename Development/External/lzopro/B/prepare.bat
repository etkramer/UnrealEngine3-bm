@call b\unset.bat
@if "%1"=="noclean" goto noclean
@call b\clean.bat
:noclean

@set CFI=-Iinclude -I.
@set BNAME=lzopro
@set BLIB=lzopro.lib
@set BDLL=lzopro.dll

@echo Compiling LZO-Professional, please be patient...
