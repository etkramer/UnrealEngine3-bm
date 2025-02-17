@echo // Copyright (C) 1996-2006 Markus F.X.J. Oberhumer
@echo //
@echo //   Windows 32-bit
@echo //   cygwin + gcc
@echo //
@call b\prepare.bat
@if "%BECHO%"=="n" echo off


set BLIB=lib%BNAME%.a
set CC=gcc
set CF=-O2 -fomit-frame-pointer -Wall %CFI% %CFASM%
set LF=%BLIB% -lwinmm -s

%CC% %CF% -Wno-uninitialized -c src/*.c
@if errorlevel 1 goto error
ar rcs %BLIB% @b/win32/cygwin.rsp
@if errorlevel 1 goto error

%CC% %CF% -o lzopack.exe examples/lzopack.c %LF%
@if errorlevel 1 goto error
%CC% %CF% -o precomp.exe examples/precomp.c %LF%
@if errorlevel 1 goto error
%CC% %CF% -o selftest.exe examples/selftest.c %LF%
@if errorlevel 1 goto error
%CC% %CF% -o simple.exe examples/simple.c %LF%
@if errorlevel 1 goto error

%CC% %CF% -o lzotest.exe lzotest/lzotest.c %LF%
@if errorlevel 1 goto error


@call b\done.bat
@goto end
:error
@echo ERROR during build!
:end
@call b\unset.bat
