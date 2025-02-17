#include "stdafx.h"
#include "XeCOMExports.h"
#include "FXenonConsole.h"

extern "C" XECOM_INTERFACE HRESULT CreateXenonConsole(IFXenonConsole **OutConsole)
{
	*OutConsole = NULL;
	CComObject<CFXenonConsole> *NewConsole = NULL;

	HRESULT Result = CComObject<CFXenonConsole>::CreateInstance(&NewConsole);
	if(SUCCEEDED(Result))
	{
		Result = NewConsole->QueryInterface(__uuidof(IFXenonConsole), (void**)OutConsole);
	}

	return Result;
}