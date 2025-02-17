REM this is meant to do any extra label updating that needs to be done for a QA label. 

REM %1 is the QA_APPROVED_BUILD_MONTH_YEAR (e.g. QA_APPROVED_BUILD_DEC_2006)
REM %2 is the UE3 build label  (e.g. UnrealEngine3_Build_[2007-03-19_15.30] )

REM so 
REM do QABuildIsGoodForLicensees QA_APPROVED_BUILD_MONTH_YEAR UnrealEngine3_Build_[date]


REM the label is already created p4 label -t labelFoo labelBar

p4 -u build_machine label -t %1
p4 -u build_machine tag -l %1 @%2
p4 -u build_machine tag -l QA_APPROVED_BUILD_CURRENT @%2


