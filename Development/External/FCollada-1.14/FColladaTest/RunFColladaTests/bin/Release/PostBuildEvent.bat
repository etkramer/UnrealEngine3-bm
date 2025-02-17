@echo off
copy C:\svn\FeelingSoftware\main\src\FCollada\FColladaTest\RunFColladaTests\bin\Release\RunFColladaTests.dll C:\svn\FeelingSoftware\main\src\FCollada\FColladaTest\RunFColladaTests\
if errorlevel 1 goto EventReportError
goto EventEnd
:EventReportError
echo Project error: A tool returned an error code from the build event
exit 1
:EventEnd
