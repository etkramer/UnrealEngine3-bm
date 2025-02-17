// MemoryProperty.cpp : Implementation of CMemoryProperty

#include "stdafx.h"
#include "MemoryProperty.h"

//////////////////////////////////////////////////////////////////////////////
// IDebugProperty2 interface
// (Most of these methods are inherited from CDebugProperty)

// Get a memory context for this property within the memory bytes returned
// by GetMemoryBytes
HRESULT CMemoryProperty::GetMemoryContext(
		IDebugMemoryContext2** ppMemory)
{
	HRESULT retval = E_POINTER;
	if (ppMemory != NULL && m_pMemoryContext != NULL) {
		retval = m_pMemoryContext->QueryInterface(__uuidof(IDebugMemoryContext2),
												  (void **)ppMemory);
	}
	return(retval);
}

//////////////////////////////////////////////////////////////////////////////
// CMemoryProperty

void CMemoryProperty::Init(BSTR pAddress,BSTR pModuleName)
{
	CComObject<CMemoryContext>::CreateInstance(&m_pMemoryContext);
	if (m_pMemoryContext != NULL) {
		m_pMemoryContext->AddRef();
		ULONGLONG address = static_cast<ULONGLONG>(_wtoi64(pAddress));
		m_pMemoryContext->Init(address);
	}
	CDebugProperty::Init(pAddress,pAddress,CComBSTR("BYTE"),true);
}
