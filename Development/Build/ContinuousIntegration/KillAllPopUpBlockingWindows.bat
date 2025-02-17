C:\bin\cygwin\bin\sleep 30

call pskill -t devenv.exe
call pskill -t vsjitdebugger.exe

call pskill -t devenv.exe
call pskill -t vsjitdebugger.exe

echo >> KillTriedToRun.txt
