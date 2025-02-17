
@echo off
python findMissingFilesInVcprojs.py *.uc ../Src
REM SET ERROR_RETVAL=%ERRORLEVEL%

exit %ERROR_RETVAL%

