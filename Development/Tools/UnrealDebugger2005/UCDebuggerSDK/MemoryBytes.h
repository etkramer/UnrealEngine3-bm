// MemoryBytes.h : Declaration of the CMemoryBytes

#pragma once
#include "resource.h"       // main symbols

#include "UCDebuggerSDK.h"



#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error Single-threaded COM objects are not properly supported on Windows CE platforms that do not include full DCOM support.  Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support them anyway. You must also change the threading model in your rgs file from 'Apartment'/'Single' to 'Free'.
#endif



// CMemoryBytes

class ATL_NO_VTABLE CMemoryBytes : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDebugMemoryBytes2
{
public:
	CMemoryBytes()
	{
	}


BEGIN_COM_MAP(CMemoryBytes)
    COM_INTERFACE_ENTRY(IDebugMemoryBytes2)
END_COM_MAP()

	STDMETHOD(ReadAt)(
		IDebugMemoryContext2* pStartContext,
		DWORD dwCount,
		BYTE* rgbMemory,
		DWORD* pdwRead,
		DWORD* pdwUnreadable);

	STDMETHOD(WriteAt)(
		IDebugMemoryContext2* pStartContext,
		DWORD dwCount,
		BYTE* rgbMemory);

	STDMETHOD(GetSize)(
		UINT64* pqwSize);


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

public:

private:
	HRESULT GetMemoryAddress(IDebugMemoryContext2 *pContext,UINT64 *pAddress);
};

