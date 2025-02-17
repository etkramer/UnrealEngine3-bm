#pragma once
void PostDebugMessage(LPCWSTR msg);
void SendCommandToUC(INT cmdId, LPCWSTR cmd);
DWORD OnCommandToVS(INT cmdId, DWORD dw1, DWORD dw2, LPCWSTR s1, LPCWSTR s2);
CComBSTR GetFileFromClass(LPCWSTR PkgClsMeth);
void PkgClsFromFile(LPCWSTR file, CComBSTR &pkg, CComBSTR &cls);
// void LogToFile(LPCTSTR pPrompt);

extern IPC_VS *g_pUCDebgger;

