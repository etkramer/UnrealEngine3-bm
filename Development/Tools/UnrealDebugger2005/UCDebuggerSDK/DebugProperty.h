// DebugProperty.h : Declaration of the CDebugProperty

#pragma once
#include "resource.h"       // main symbols

#include "UCDebuggerSDK.h"



#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error Single-threaded COM objects are not properly supported on Windows CE platforms that do not include full DCOM support.  Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support them anyway. You must also change the threading model in your rgs file from 'Apartment'/'Single' to 'Free'.
#endif


//////////////////////////////////////////////////////////////////////////////
// CDebugProperty
// Base class for all classes that specialize IDebugProperty2.

class ATL_NO_VTABLE CDebugProperty : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDebugProperty2
{
public:
	CDebugProperty()
	{
		m_dwAttrib = DBG_ATTRIB_DATA;
		m_pParent = NULL;
	}


BEGIN_COM_MAP(CDebugProperty)
    COM_INTERFACE_ENTRY(IDebugProperty2)
END_COM_MAP()

	// Get the DEBUG_PROPERTY_INFO that describes this property
	STDMETHOD(GetPropertyInfo)(
		DEBUGPROP_INFO_FLAGS dwFields,
		DWORD dwRadix,
		DWORD dwTimeout,
		IDebugReference2** rgpArgs,
		DWORD dwArgCount,
		DEBUG_PROPERTY_INFO* pPropertyInfo);

	// Set the value of this property
	STDMETHOD(SetValueAsString)(
		LPCOLESTR pszValue,
		DWORD dwRadix,
		DWORD dwTimeout);

	// Set the value of this property
	STDMETHOD(SetValueAsReference)(
		IDebugReference2** rgpArgs,
		DWORD dwArgCount,
		IDebugReference2* pValue,
		DWORD dwTimeout);

	// Enum the children of this property
	STDMETHOD(EnumChildren)(
		DEBUGPROP_INFO_FLAGS dwFields,
		DWORD dwRadix,
		REFGUID guidFilter,
		DBG_ATTRIB_FLAGS dwAttribFilter,
		LPCOLESTR pszNameFilter,
		DWORD dwTimeout,
		IEnumDebugPropertyInfo2** ppEnum);

	// Get the parent of this property
	STDMETHOD(GetParent)(
		IDebugProperty2** ppParent);

	// Get the property that describes the derived most property of this property
	STDMETHOD(GetDerivedMostProperty)(
		IDebugProperty2** ppDerivedMost);

	// Get the memory bytes that contains this property
	STDMETHOD(GetMemoryBytes)(
		IDebugMemoryBytes2** ppMemoryBytes);

	// Get a memory context for this property within the memory bytes returned by GetMemoryBytes
	STDMETHOD(GetMemoryContext)(
		IDebugMemoryContext2** ppMemory);

	// Get the size (in bytes) of this property
	STDMETHOD(GetSize)(
		DWORD* pdwSize);

	// Get a reference for this property
	STDMETHOD(GetReference)(
		IDebugReference2** ppReference);

	// Get extended info for this property
	STDMETHOD(GetExtendedInfo)(
		REFGUID guidExtendedInfo,
		VARIANT* pExtendedInfo);


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
public:
	void Init(BSTR pName, BSTR pValue, BSTR pType, bool bIsReadOnly = false);
	void SetParent(CDebugProperty *pParent);

protected:
	CComBSTR          m_bstrName;
	CComBSTR          m_bstrValue;
	CComBSTR          m_bstrType;
	CDebugProperty   *m_pParent;
	DBG_ATTRIB_FLAGS  m_dwAttrib;
};

