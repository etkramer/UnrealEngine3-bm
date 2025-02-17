
REM this will update the build summary page

REM %1 is the CIS code builder changelist which passed


sqlcmd -S DB-01 -d "Perf_Build" -E -Q "UPDATE Commands SET LastGoodChangeList=%1 WHERE ( Description = 'CIS PC' )"
sqlcmd -S DB-01 -d "Perf_Build" -E -Q "UPDATE Commands SET LastGoodChangeList=%1 WHERE ( Description = 'CIS PS3' )"
sqlcmd -S DB-01 -d "Perf_Build" -E -Q "UPDATE Commands SET LastGoodChangeList=%1 WHERE ( Description = 'CIS Xenon' )"

