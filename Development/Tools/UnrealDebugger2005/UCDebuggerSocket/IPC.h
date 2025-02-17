#pragma once
#include "WTGlobals.h"


typedef DWORD (*OnCommandToUCProto)(INT, LPCWSTR);
typedef DWORD (*OnCommandToVSProto)(INT, DWORD, DWORD, LPCWSTR, LPCWSTR);
extern OnCommandToVSProto OnCommandVS;
extern OnCommandToUCProto OnCommandUC;

