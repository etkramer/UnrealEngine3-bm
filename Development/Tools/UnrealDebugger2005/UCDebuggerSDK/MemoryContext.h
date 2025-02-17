// MemoryContext.h : Declaration of the CMemoryContext

#pragma once
#include "resource.h"       // main symbols

#include "UCDebuggerSDK.h"



#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error Single-threaded COM objects are not properly supported on Windows CE platforms that do not include full DCOM support.  Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support them anyway. You must also change the threading model in your rgs file from 'Apartment'/'Single' to 'Free'.
#endif



// CMemoryContext

class ATL_NO_VTABLE CMemoryContext : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDebugMemoryContext2
{
public:
	CMemoryContext()
	{
		ZeroMemory(&m_contextInfo,sizeof(m_contextInfo));
	}


BEGIN_COM_MAP(CMemoryContext)
    COM_INTERFACE_ENTRY(IDebugMemoryContext2)
END_COM_MAP()


	STDMETHOD(GetName)(
		BSTR* pbstrName);

	STDMETHOD(GetInfo)(
		CONTEXT_INFO_FIELDS dwFields,
		CONTEXT_INFO* pInfo);

	STDMETHOD(Add)(
		UINT64 dwCount,
		IDebugMemoryContext2** ppMemCxt);

	STDMETHOD(Subtract)(
		UINT64 dwCount,
		IDebugMemoryContext2** ppMemCxt);

	STDMETHOD(Compare)(
		CONTEXT_COMPARE compare,
		IDebugMemoryContext2** rgpMemoryContextSet,
		DWORD dwMemoryContextSetLen,
		DWORD* pdwMemoryContext);

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		FreeContextInfo();
	}

public:
	void Init(UINT64 address);

protected:
	void FreeContextInfo();
	HRESULT CreateNewContextFromAddress(UINT64 address,IDebugMemoryContext2** ppMemCxt);

protected:
	UINT64 m_address;
	CONTEXT_INFO m_contextInfo;
};

