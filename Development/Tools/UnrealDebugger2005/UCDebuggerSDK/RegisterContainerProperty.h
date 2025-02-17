#pragma once
#include "containerproperty.h"

class CRegisterContainerProperty :
	public CContainerProperty
{
public:
	CRegisterContainerProperty(void);
	~CRegisterContainerProperty(void);

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void Init();
};

