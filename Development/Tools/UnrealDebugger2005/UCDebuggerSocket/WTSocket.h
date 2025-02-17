#pragma once
#include "Sockets.h"


class WTSocketCommand : public SocketSender
{
public:
	virtual LPCWSTR Read();
	void Send(LPCWSTR cmdStr = NULL);	
	void ConnectClient();

};
