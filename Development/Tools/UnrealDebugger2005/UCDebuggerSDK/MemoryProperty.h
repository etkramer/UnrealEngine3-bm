// MemoryProperty.h : Declaration of the CMemoryProperty

#pragma once
#include "resource.h"       // main symbols

#include "UCDebuggerSDK.h"
#include "DebugProperty.h"
#include "MemoryContext.h"
#include "MemoryBytes.h"


#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error Single-threaded COM objects are not properly supported on Windows CE platforms that do not include full DCOM support.  Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support them anyway. You must also change the threading model in your rgs file from 'Apartment'/'Single' to 'Free'.
#endif



// CMemoryProperty

class CMemoryProperty :
	public CDebugProperty
{
public:
	CMemoryProperty()
	{
	}


	// Get a memory context for this property within the memory bytes returned by GetMemoryBytes
	STDMETHOD(GetMemoryContext)(
		IDebugMemoryContext2** ppMemory);


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		if (m_pMemoryContext != NULL) {
			m_pMemoryContext->Release();
			m_pMemoryContext = NULL;
		}
	}

public:

	void Init(BSTR pAddress,BSTR pModuleName);
protected:
	CComObject<CMemoryContext> *m_pMemoryContext;
};
