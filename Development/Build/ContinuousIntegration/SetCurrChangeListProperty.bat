
REM this will find the current changelist and return it

for /f "tokens=3" %%i in ('p4 -ztag changes -m1 -s submitted //... ^|findstr change') do exit %%i

