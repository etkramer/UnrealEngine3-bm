// RegisterProperty.h : Declaration of the CRegisterProperty

#pragma once
#include "resource.h"       // main symbols

#include "UCDebuggerSDK.h"
#include "DebugProperty.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error Single-threaded COM objects are not properly supported on Windows CE platforms that do not include full DCOM support.  Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support them anyway. You must also change the threading model in your rgs file from 'Apartment'/'Single' to 'Free'.
#endif



// CRegisterProperty

class CRegisterProperty : 
	public CDebugProperty
{
public:
	CRegisterProperty()
	{
	}

	// Set the value of this property
	STDMETHOD(SetValueAsString)(
		LPCOLESTR pszValue,
		DWORD dwRadix,
		DWORD dwTimeout);


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
public:
	void Init(BSTR registerGroupName,ULONG registerIndex,BSTR registerName,BSTR value);

protected:
	CComBSTR m_bstrRegisterGroupName;
	ULONG    m_ulRegisterIndex;
};
