#pragma once
#include "StdAfx.h"

class UCSocketCommand : public SocketSender
{
public:
	UCSocketCommand();
	void Send( INT cmdId, DWORD dw1 = 0, DWORD dw2 = 0, LPCWSTR s1 = NULL, LPCWSTR s2 = NULL );
	virtual LPCWSTR Read();
	~UCSocketCommand();
	void StartVSDebugger();
};
