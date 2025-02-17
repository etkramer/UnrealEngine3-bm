@ECHO OFF
SET LOG=%TEMP%\UnrealSyncInstall.txt
net stop UnrealSync> %LOG% 2>&1
ECHO Uninstalling previous versions...
%WINDIR%\Microsoft.NET\Framework\v2.0.50727\installUtil.exe /u "%PROGRAMFILES%\Epic Games\USync\USyncService.exe"> %LOG% 2>&1
%WINDIR%\Microsoft.NET\Framework\v2.0.50727\installUtil.exe /u "%PROGRAMFILES%\Epic Games\UnrealSync\UnrealSyncService.exe"> %LOG% 2>&1
rmdir "%PROGRAMFILES%\Epic Games\USync" /S /Q> %LOG% 2>&1
rmdir "%PROGRAMFILES%\Epic Games\UnrealSync" /S /Q> %LOG% 2>&1
del "%USERPROFILE%\Desktop\USync Manager.lnk" /Q> %LOG% 2>&1
del "%USERPROFILE%\Desktop\UnrealSync Manager.lnk" /Q> %LOG% 2>&1
ECHO.
ECHO Creating the UnrealSync program folder...
mkdir "%PROGRAMFILES%\Epic Games\UnrealSync\"> %LOG% 2>&1
ECHO.
ECHO Copying the application files...
copy "x:\Internal Tools\UnrealSync\*.exe" "%PROGRAMFILES%\Epic Games\UnrealSync\"> %LOG% 2>&1
copy "x:\Internal Tools\UnrealSync\*.dll" "%PROGRAMFILES%\Epic Games\UnrealSync\"> %LOG% 2>&1
copy "*.lnk" "%USERPROFILE%\Desktop\"> %LOG% 2>&1
ECHO.
ECHO Installing the service...
ECHO Enter your domain username as epicgames\first.last
%WINDIR%\Microsoft.NET\Framework\v2.0.50727\installUtil.exe "%PROGRAMFILES%\Epic Games\UnrealSync\UnrealSyncService.exe"> %LOG% 2>&1
ECHO.
ECHO Starting Service...
net start UnrealSync
ECHO Cleaning up...
ECHO.
ECHO Installation Complete!
ECHO.
pause